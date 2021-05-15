#!/bin/sh

if [ ! -x /usr/bin/openssl ] && [ ! -x /opt/bin/openssl ] ; then
  echo "Unable to find the 'openssl' executable!"
  echo "Please install 'openssl-util' package from Entware."
  exit 1
fi

umask 0022

## path to openvpn binary
OPENVPN=/usr/sbin/openvpn
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
STC2_KEY=stc2.key
ECPARAM=ecparam.pem
## number of bits to use when generate new key
RSA_BITS=1024
## number of bits for prime
DH_BITS=1024
## CA common name
CA_CN="OpenVPN CA"
## temp config file with cert extensions
SSL_EXT_FILE=/etc/ssl/openssl_ext1.cnf
## certs storage path
CRT_PATH=/etc/storage/openvpn
CRT_PATH_SRV="$CRT_PATH/server"
if [ -z "$CRT_PATH_CLI" ] ; then
  CRT_PATH_CLI="$CRT_PATH/client"
fi

# Check if -sha256 is supported
DGST_ALG="-sha1"
openssl list -1 --digest-commands 2>&1 | grep -q 'sha256' && DGST_ALG="-sha256"

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
  echo "    `$BOLD`server`$NORM` [ -n `$BOLD`common_name`$NORM` ] [ -b `$BOLD`rsa_bits/ec_name`$NORM` ] [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           The following files for OpenVPN server are created:" >&2
  echo "           - root CA key and certificate" >&2
  echo "           - server key and certificate" >&2
  echo "           - Diffie-Hellman parameters key" >&2
  echo "           - TLS-Auth HMAC signature key" >&2
  echo "           `$BOLD`Note:`$NORM` $CA_CRT and ${TA_KEY}(if TLS-Auth or TLS-Crypt is used) should be sent to clients." >&2
  echo >&2
  echo "    `$BOLD`client`$NORM` -n `$BOLD`common_name`$NORM` [ -b `$BOLD`rsa_bits/ec_name`$NORM` ] [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           Create both client key and sign it on server side. It is not quite correct," >&2
  echo "           but it saves time if you administer both server and client devices." >&2
  echo >&2
  echo "    `$BOLD`client_csr`$NORM` -n `$BOLD`common_name`$NORM` [ -b `$BOLD`rsa_bits/ec_name`$NORM` ]" >&2
  echo "           The following files for OpenVPN client are created:" >&2
  echo "           - client key" >&2
  echo "           - certificate signing request (client.csr)" >&2
  echo "           `$BOLD`Note:`$NORM` This request should be signed with OpenVPN server CA certificate." >&2
  echo >&2
  echo "    `$BOLD`client_sign`$NORM` -f `$BOLD`csr_file_path`$NORM` [ -d `$BOLD`days_valid`$NORM` ]" >&2
  echo "           Create client certificate." >&2
  echo >&2
  echo "    `$BOLD`ssl_view`$NORM` -f `$BOLD`crt/csr_file_path`$NORM`" >&2
  echo "           Allows you to see the contents of the requests or certificates using the" >&2
  echo "           `$BOLD`openssl`$NORM` utility." >&2
  echo >&2
  echo >&2
  echo "`$BOLD`Example:`$NORM`" >&2
  echo "  If you are new to OpenVPN but want to connect server and client," >&2
  echo "  you can create certificates using:" >&2
  echo "    `$BOLD`$0 server`$NORM`" >&2
  echo "    `$BOLD`$0 client -n client1`$NORM`" >&2
  echo "  Then copy the following files to client:" >&2
  echo "    `$BOLD`$CA_CRT`$NORM`, `$BOLD`$TA_KEY`$NORM` from `$BOLD`$CRT_PATH_SRV`$NORM`" >&2
  echo "    `$BOLD`$CLIENT_KEY`$NORM`, `$BOLD`$CLIENT_CRT`$NORM` from `$BOLD`$CRT_PATH_CLI`$NORM`" >&2
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
  ssl_view) ;;
  *) func_help ;;
esac

[ $(( $# % 2 )) -ne 0 ] && func_help

while [ $# -gt 0 ] ; do
  case "$1" in
    -b) RSA_BITS=$2   ; shift 2 ;;
    -d) CERT_DAYS=$2  ; shift 2 ;;
    -n) CN=$2         ; shift 2 ;;
    -f) CSR_PATH=$2   ; shift 2 ;;
    -s) CRL_SERIAL=$2 ; shift 2 ;;
     *) func_help     ;;
  esac
done


write_ext_cfs() {
  rm -f $SSL_EXT_FILE
  cat > $SSL_EXT_FILE << EOF
[ server ]
extendedKeyUsage=serverAuth
keyUsage=critical,digitalSignature,keyEncipherment
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
  # $4 --> rsa bits
  # $5 --> CN
  # $6 --> signature algorithm
  # $7 --> ca if cert is CA
  #
  [ "$7" == "ca" ] && local CA_TRUE="-x509"
    if  [ ! -s $1 ] ; then
         [[ `echo $4 | grep '^[bpsw]'` ]] && openssl ecparam -name $4 -genkey -out $1
         [[ `echo $4 | grep '^ed'` ]] && openssl genpkey -algorithm $4 -out $1
         [[ `echo $4 | grep '^[1-9]'` ]] && openssl genrsa -out $1 $4
    fi
    echo_process "Creating ${2}: $5"
    openssl req -nodes $CA_TRUE -days $3 -new -outform PEM \
            -out $2 -key $1 $6 -subj "/CN=$5" &>/dev/null
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
  # $6 --> signature algorithm
  # $7 --> extensions to use (server or client)
  #
  if [ ! -f $1 ] || [ ! -f $2 ] ; then
    echo "Error: CA not found" >&2
    return 1
  fi
  write_ext_cfs
  echo_process "Signing $4"
  openssl x509 -req -in $3 -CA $2 -CAkey $1 -CAcreateserial \
               -clrext -out $4 -$6 -extfile $SSL_EXT_FILE \
               -days $5 -extensions $7 &>/dev/null
  rm -f $3
  rm -f ca.srl
  echo_done
}

make_dh() {
  #
  # $1 --> dh bits
  #
  if [ -f dh1024.pem ] ; then
    echo_process "Skipping DH Parameters. File exists"
  else
    echo_process "Creating DH Parameters (may take long time, be patient)"
    openssl dhparam -out dh1024.pem $1 &>/dev/null
  fi
  echo_done
}

make_ta_tc2() {
  #
  # $1 --> ta key name
  #
  # $2 --> server tc2 key name
  #

  if [ ! -x $OPENVPN ] ; then
    echo_process "Skipping TLS Auth/Crypt key. $OPENVPN not found."
    echo_done
    return 1
  fi
  if [ -f $1 ] ; then
    echo_process "Skipping TLS Auth/Crypt key. File exists"
    echo_done
  else
    echo_process "Creating TLS Auth/Crypt key"
    $OPENVPN --genkey secret $1 &>/dev/null
    [ -s $1 ] && chmod 600 $1
    echo_done
  fi
  if [ -f $2 ] ; then
    echo_process "Skipping TLS Crypt v2 server key. File exists"
    echo_done
  else
    echo_process "Creating TLS Crypt v2 server key"
    $OPENVPN --genkey tls-crypt-v2-server $2 &>/dev/null
    [ -s $2 ] && chmod 644 $2
    echo_done
  fi
}

server() {
  local CRT_PATH_X=$CRT_PATH_SRV
  [ -d $CRT_PATH_X ] || mkdir -m 755 -p $CRT_PATH_X
  if ! cd $CRT_PATH_X ; then
    echo "Error: can't cd to $CRT_PATH_X" >&2
    return 1
  fi
  ## Create CA
  make_cert $CA_KEY $CA_CRT $CA_DAYS $RSA_BITS "$CA_CN" $DGST_ALG ca
  ## Create server csr
  [ -z "$CN" ] && CN="OpenVPN Server"
  make_cert $SERVER_KEY server.csr $CERT_DAYS $RSA_BITS "$CN" $DGST_ALG
  ## Sign server csr
  sign_cert $CA_KEY $CA_CRT server.csr $SERVER_CRT $CERT_DAYS $DGST_ALG server
  ## Create DH param
  make_dh $DH_BITS
  ## Create TLS Auth/Crypt key and TLS Crypt v2 server key
  make_ta_tc2 $TA_KEY $STC2_KEY
}

client_csr() {
  local CRT_PATH_X=$CRT_PATH_CLI
  [ -d $CRT_PATH_X ] || mkdir -m 755 -p $CRT_PATH_X
  if ! cd $CRT_PATH_X ; then
    echo "Error: can't cd to $CRT_PATH_X" >&2
    return 1
  fi
  [ -z "$CN" ] && func_help
  make_cert $CLIENT_KEY client.csr $CERT_DAYS $RSA_BITS "$CN" $DGST_ALG
}

client_sign() {
  [ -z "$CSR_PATH" ] && func_help
  if [ "${CSR_PATH:0:1}" != "/" ] ; then
    CSR_PATH=`pwd`/$CSR_PATH
  fi
  local CRT_PATH_X=$CRT_PATH_SRV
  [ -d $CRT_PATH_X ] || mkdir -m 755 -p $CRT_PATH_X
  if ! cd $CRT_PATH_X ; then
    echo "Error: can't cd to $CRT_PATH_X" >&2
    return 1
  fi
  if [ ! -f $CSR_PATH ] ; then
    echo "Error: $CSR_PATH - file not found" >&2
    return 1
  fi
  sign_cert $CA_KEY $CA_CRT $CSR_PATH ${CSR_PATH%.*}.crt $CERT_DAYS $DGST_ALG client
}

client() {
  client_csr
  CSR_PATH="$CRT_PATH_CLI/client.csr"
  client_sign
}

ssl_view () {
  [ -z "$CSR_PATH" ] && func_help
case ${CSR_PATH##*.} in
	crt) openssl x509 -in "$CSR_PATH" -noout -text ;;
	csr) openssl req -in "$CSR_PATH" -noout -text ;;
	*)   func_help ;;
esac
}

eval $ACTION
