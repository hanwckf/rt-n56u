#!/bin/sh

if [ ! -x /usr/bin/openssl ] && [ ! -x /opt/bin/openssl ] ; then
  echo "Unable to find the 'openssl' executable!"
  echo "Please install 'openssl-util' package from Entware."
  exit 1
fi

## CA cert valid, days
CA_DAYS=3653
## default value for certs valid period, days
CERT_DAYS=365
## cert and key file names
CA_CRT=ca.crt
CA_KEY=ca.key
SERVER_KEY=server.key
SERVER_CRT=server.crt
CLIENT_KEY=client.key
CLIENT_CRT=client.crt
TA_KEY=ta.key
## number of bits to use when generate new key
KEY_BITS=1024
## number of bits for prime
DH_BITS=1024
## CA common name
CA_CN="OpenVPN CA"
## temp config file with cert extensions
SSL_EXT_FILE=/tmp/openssl_ext.cnf
## certs storage path
CRT_PATH=/etc/storage/openvpn
## path to openvpn binary
OPENVPN=/usr/sbin/openvpn

func_help() {
  local BOLD="echo -ne \\033[1m"
  local NORM="echo -ne \\033[0m"
  echo >&2
  echo "Create certificates for OpenVPN client/server. For more info see:" >&2
  echo "http://openvpn.net/index.php/open-source/documentation/howto.html" >&2
  echo "https://code.google.com/p/rt-n56u/wiki/HowToConfigureOpenvpnServer" >&2
  echo >&2
  echo "`$BOLD`Usage:`$NORM` $0 command [ args ]" >&2
  echo >&2
  echo "    `$BOLD`commands:`$NORM` [ server, client, client_csr, client_sign ]" >&2
  echo >&2
  echo "    `$BOLD`server`$NORM` [ -n `$BOLD`common_name`$NORM` ] [ -b `$BOLD`num_key_bits`$NORM` ] [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           The following files for OpenVPN server are created:" >&2
  echo "           - root CA key and certificate" >&2
  echo "           - server key and certificate" >&2
  echo "           - Diffie-Hellman parameters key" >&2
  echo "           - TLS-Auth HMAC signature key" >&2
  echo "           `$BOLD`Note:`$NORM` $CA_CRT and ${TA_KEY}(if TLS-Auth is used) should be sent to clients." >&2
  echo >&2
  echo "    `$BOLD`client`$NORM` -n `$BOLD`common_name`$NORM` [ -b `$BOLD`num_key_bits`$NORM` ] [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           Create both client key and sign it on server side. It is not quite correct," >&2
  echo "           but it saves time if you administer both server and client devices." >&2
  echo >&2
  echo "    `$BOLD`client_csr`$NORM` -n `$BOLD`common_name`$NORM` [ -b `$BOLD`num_key_bits`$NORM` ]" >&2
  echo "           The following files for OpenVPN client are created:" >&2
  echo "           - client key" >&2
  echo "           - certificate signing request (client.csr)" >&2
  echo "           `$BOLD`Note:`$NORM` This request should be signed with OpenVPN server CA certificate." >&2
  echo >&2
  echo "    `$BOLD`client_sign`$NORM` -f `$BOLD`csr_file_path`$NORM` [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           Create client certificate." >&2
  echo >&2
  echo >&2
  echo "`$BOLD`Example:`$NORM`" >&2
  echo "  If you are new to OpenVPN but want to connect server and client," >&2
  echo "  you can create certificates using:" >&2
  echo "    `$BOLD`$0 server`$NORM`" >&2
  echo "    `$BOLD`$0 client -n client1`$NORM`" >&2
  echo "  Then copy the following files to client:" >&2
  echo "    `$BOLD`$CA_CRT`$NORM`, `$BOLD`$TA_KEY`$NORM` from `$BOLD`$CRT_PATH/server`$NORM`" >&2
  echo "    `$BOLD`$CLIENT_KEY`$NORM`, `$BOLD`$CLIENT_CRT`$NORM` from `$BOLD`$CRT_PATH/client`$NORM`" >&2
  echo >&2
  exit 1
}

ACTION=$1
shift
case "$ACTION" in
  client) ;;
  client_csr) ;;
  client_sign) ;;
  server) ;;
  *) func_help ;;
esac

[ $(( $# % 2 )) -ne 0 ] && func_help

while [ $# -gt 0 ] ; do
  case "$1" in
    -b) KEY_BITS=$2   ; shift 2 ;;
    -d) CERT_DAYS=$2  ; shift 2 ;;
    -n) CN=$2         ; shift 2 ;;
    -f) CSR_PATH=$2   ; shift 2 ;;
    -s) CRL_SERIAL=$2 ; shift 2 ;;
    *)  func_help     ;;
  esac
done


write_ext_cfs() {
  [ -s $SSL_EXT_FILE ] && return 0
  cat > $SSL_EXT_FILE << EOF
[ server ]
extendedKeyUsage=serverAuth
keyUsage=critical,digitalSignature, keyEncipherment
[ client ]
extendedKeyUsage=clientAuth
keyUsage=critical,digitalSignature
EOF
}

echo_done() {
  echo -e " *\033[60G [ done ]"
}

echo_process() {
  echo -ne " + $@\r"
}

make_cert() {
  #
  # $1 --> key file name
  # $2 --> cert file name
  # $3 --> days valid
  # $4 --> key bits
  # $5 --> CN
  # $6 --> ca if cert is CA
  #
  [ "$6" == "ca" ] && local CA_TRUE="-x509"
  if [ -s $1 ] ; then
    echo_process "Creating ${2}: $5"
    openssl req -nodes $CA_TRUE -days $3 -new -outform PEM \
            -out $2 -key $1 -sha1 -subj "/CN=$5" &>/dev/null
  else
    echo_process "Creating new ${2}: $5"
    openssl req -nodes $CA_TRUE -days $3 -newkey rsa:$4 \
            -outform PEM -out $2 -keyout $1 -sha1 -subj "/CN=$5" &>/dev/null
  fi
  [ -f $1 ] && chmod 600 $1
  echo_done
}

sign_cert() {
  #
  # $1 --> ca key file name
  # $2 --> ca crt file name
  # $3 --> csr input file name
  # $4 --> crt output file name
  # $5 --> days valid
  # $6 --> extensions to use (server or client)
  #
  if [ ! -f $1 ] || [ ! -f $2 ] ; then
    echo "Error: CA not found" >&2
    return 1
  fi
  write_ext_cfs
  echo_process "Signing $4"
  openssl x509 -req -in $3 -CA $2 -CAkey $1 -CAcreateserial \
               -clrext -out $4 -sha1 -extfile $SSL_EXT_FILE \
               -days $5 -extensions $6 &>/dev/null
  rm -f $3
  echo_done
}

make_dh() {
  #
  # $1 --> num bits
  #
  if [ -f dh${1}.pem ] ; then
    echo_process "Skipping DH Parameters. File exists"
    echo_done
    return
  fi
  echo_process "Creating DH Parameters (may take long time, be patient)"
  openssl dhparam -out dh${1}.pem $1 &>/dev/null
  echo_done
}

make_ta() {
  #
  # $1 --> ta key name
  #
  if [ ! -x $OPENVPN ] ; then
    echo_process "Skipping TLS Auth key. $OPENVPN not found."
    echo_done
    return 1
  fi
  if [ -f $1 ] ; then
    echo_process "Skipping TLS Auth key. File exists"
    echo_done
    return 0
  fi
  echo_process "Creating TLS Auth key"
  $OPENVPN --genkey --secret $1 &>/dev/null
  [ -s $1 ] && chmod 600 $1
  echo_done
}

server() {
  local CRT_PATH_X=$CRT_PATH/server
  [ -d $CRT_PATH_X ] || mkdir -m 755 -p $CRT_PATH_X
  if ! cd $CRT_PATH_X ; then
    echo "Error: can't cd to $CRT_PATH_X" >&2
    return 1
  fi
  ## Create CA
  make_cert $CA_KEY $CA_CRT $CA_DAYS $KEY_BITS "$CA_CN" ca
  ## Create server csr
  [ -z "$CN" ] && CN="OpenVPN Server"
  make_cert $SERVER_KEY server.csr $CERT_DAYS $KEY_BITS "$CN"
  ## Sign server csr
  sign_cert $CA_KEY $CA_CRT server.csr $SERVER_CRT $CERT_DAYS server
  ## Create DH key
  make_dh $DH_BITS
  ## Create TLS Auth key
  make_ta $TA_KEY
  ## Cleanup
  rm -f ca.srl
}

client_csr() {
  local CRT_PATH_X=$CRT_PATH/client
  [ -d $CRT_PATH_X ] || mkdir -m 755 -p $CRT_PATH_X
  if ! cd $CRT_PATH_X ; then
    echo "Error: can't cd to $CRT_PATH_X" >&2
    return 1
  fi
  [ -z "$CN" ] && func_help
  make_cert $CLIENT_KEY client.csr $CERT_DAYS $KEY_BITS "$CN"
}

client_sign() {
  [ -z "$CSR_PATH" ] && func_help
  if [ "${CSR_PATH:0:1}" != "/" ] ; then
    CSR_PATH=`pwd`/$CSR_PATH
  fi
  local CRT_PATH_X=$CRT_PATH/server
  [ -d $CRT_PATH_X ] || mkdir -m 755 -p $CRT_PATH_X
  if ! cd $CRT_PATH_X ; then
    echo "Error: can't cd to $CRT_PATH_X" >&2
    return 1
  fi
  if [ ! -f $CSR_PATH ] ; then
    echo "Error: $CSR_PATH - file not found" >&2
    return 1
  fi
  sign_cert $CA_KEY $CA_CRT $CSR_PATH ${CSR_PATH%.*}.crt $CERT_DAYS client
}

client() {
  client_csr
  CSR_PATH=$CRT_PATH/client/client.csr
  client_sign
}

eval $ACTION
