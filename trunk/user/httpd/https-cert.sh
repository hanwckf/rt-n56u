#!/bin/sh

if [ ! -x /usr/bin/openssl ] && [ ! -x /opt/bin/openssl ]; then
  echo "Unable to find the 'openssl' executable!" >&2
  echo "Please install 'openssl-util' package from Entware." >&2
  exit 1
fi

DSTDIR=/etc/storage/https
BITS=1024
DAYS=365
SSL_EXT_FILE=/tmp/openssl_ext.cfg

func_help() {
  local BOLD="echo -ne \\033[1m"
  local NORM="echo -ne \\033[0m"
  echo "Create self-signed certificate for HTTP SSL server." >&2
  echo >&2
  echo "`$BOLD`Usage:`$NORM` $0 -n cert_common_name [ -b key_bits ] [ -d days_valid ]" >&2
  echo >&2
  echo "`$BOLD`Example:`$NORM`" >&2
  echo "    $0 -n myname.no-ip.com -b 2048 -d 3650" >&2
  echo >&2
  echo "`$BOLD`Defaults:`$NORM`"
  echo "    key_bits=`$BOLD`1024`$NORM`, days_valid=`$BOLD`365`$NORM`" >&2
  echo >&2
  exit 1
}

echo_done() {
  echo -e " *\033[60G [ done ]"
}

echo_process() {
  echo -ne " + $@\r"
}

is_cn_valid() {
  # RFC 1034 (https://tools.ietf.org/html/rfc1034#section-3.5)
  if [ -n "`echo $1 | sed 's/^[a-zA-Z][0-9a-zA-z\.-]\{1,253\}[0-9a-zA-Z]$//'`" ] ; then
    return 1
  fi
  for i in  `echo $1 | sed 's/\./ /g'` ; do
    if [ `echo -n $i | wc -c` -gt 63 ] ; then
      return 1
    fi
  done
  return 0
}

create_cert() {
  [ -z "$CN" ] && func_help
  if ! is_cn_valid $CN ; then
     echo >&2
     echo "Warning: $CN is not a valid host name." >&2
     echo >&2
  fi

  [ -d $DSTDIR ] || mkdir -m 755 -p $DSTDIR
  if ! cd $DSTDIR ; then
    echo "Error: can't cd to $DSTDIR" >&2
    return 1
  fi

  # Check if -sha256 is supported
  local DGST_ALG
  openssl dgst -h 2>&1 | grep -q '^-sha256' && DGST_ALG="-sha256" || DGST_ALG="-sha1"

  cat > $SSL_EXT_FILE << EOF
[ server ]
basicConstraints=CA:FALSE
subjectKeyIdentifier=hash
authorityKeyIdentifier=keyid,issuer:always
extendedKeyUsage=serverAuth,clientAuth
keyUsage=critical,digitalSignature,keyEncipherment
subjectAltName=DNS:${CN},DNS:my.router
EOF

    echo_process "Creating CA"
    openssl req -nodes -x509 -days $(($DAYS * 10 + 3)) -newkey rsa:${BITS} \
            -outform PEM -out ca.crt -keyout ca.key \
            $DGST_ALG -subj "/CN=HTTPS CA" &>/dev/null
    chmod 600 ca.key
    echo_done

    echo_process "Creating certificate request"
    openssl req -nodes -days $DAYS -newkey rsa:${BITS} \
            -outform PEM -out server.csr -keyout server.key \
            $DGST_ALG -subj "/CN=$CN" &>/dev/null
    chmod 600 server.key
    echo_done

    echo_process "Signing request"
    openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
               -clrext -out server.crt $DGST_ALG -extfile $SSL_EXT_FILE \
               -days $DAYS -extensions server &>/dev/null
    [ -f server.crt ] && rm -f server.csr
    echo_done

    echo_process "Creating DH Parameters (may take long time, be patient)"
    openssl dhparam -out dh1024.pem 1024 &>/dev/null
    echo_done

    [ -f $SSL_EXT_FILE ] && rm -f $SSL_EXT_FILE
    [ -f ca.srl ] && rm -f ca.srl
}

[ $(( $# % 2 )) -ne 0 ] && func_help

while [ $# -gt 0 ] ; do
  case "$1" in
    -n) CN="$2" ; shift 2 ;;
    -b) BITS=$2 ; shift 2 ;;
    -d) DAYS=$2 ; shift 2 ;;
    *)  func_help ;;
  esac
done

create_cert

exit $?
