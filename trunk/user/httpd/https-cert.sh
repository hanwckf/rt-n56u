#!/bin/sh

if [ ! -x /usr/bin/openssl ] && [ ! -x /opt/bin/openssl ]; then
	echo "Unable to find the 'openssl' executable!"
	echo "Please install 'openssl-util' package from Entware."
	exit 1
fi

################################################################################
indir=$(pwd)
ssldir=/etc/ssl
keydir=/etc/ssl/keys
dstdir=/etc/storage/https

################################################################################
do_check_cfg()
{
## Create default directory structure according to config
[ -d ./private ]     || { mkdir ./private; chmod 700 ./private; }
[ -d ./crl ]         ||   mkdir ./crl
[ -d ./certs ]       ||   mkdir ./certs
[ -d ./newcerts ]    ||   mkdir ./newcerts
[ -f ./index ]       ||   touch ./index
[ -f ./serial ]      || { touch ./serial ; echo 01 > ./serial ; }
}

################################################################################
## reads values for certificate/key and saves them to local file in order not to
## ask user each time he needs do create a certificate/key.
do_get_raw_data()
{
http_ipaddr=`nvram get lan_ipaddr_t`
clear

GETVAL=1

echo -e "\033[1m"
cat <<EOF

================================================================================

 You are about to be asked to enter information that will be incorporated
 into your certificate request.

 If you'd like to choose default value, just press 'Enter' (leave field blank).
 If you want a field be blank (contain no data), type '.' and press 'Enter'

================================================================================




EOF
echo -e "\033[0m"


while [ $GETVAL -eq 1 ] ; do

chck_val=1
while [ $chck_val -eq 1 ] ; do
read -p "Enter country name ( 2 letter code ) [ `nvram get wl_country_code` ] : " COUNTRY
COUNTRY=$(echo $COUNTRY | sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/; s/[^A-Z]//g')
[ $(( `echo "$COUNTRY" | wc -m` - 1 )) -eq 2 ] && chck_val=0
[ $(( `echo "$COUNTRY" | wc -m` - 1 )) -eq 0 ] && chck_val=0
[ "$COUNTRY" == "." ] && chck_val=0
done

read -p "Enter province ( some state/region ) [ ] : " PROVINCE
PROVINCE=$(echo $PROVINCE | sed 's/[^\.a-zA-Z0-9_-]//g')

read -p "Enter city [ ] : " CITY
CITY=$(echo $CITY | sed 's/[^\.a-zA-Z0-9_-]//g')

read -p "Enter organization [ `nvram get computer_name` ] : " ORG
ORG=$(echo $ORG | sed 's/[^\.a-zA-Z0-9_-]//g')

read -p "Enter organization unit [ ] : " OU
OU=$(echo $OU | sed 's/[^\.a-zA-Z0-9_-]//g')

chck_val=1
while [ $chck_val -eq 1 ] ; do
read -p "Enter Common Name ( server hostname ) [ $http_ipaddr ] : " CN
CN=$(echo $CN | sed 's/[^\.a-zA-Z0-9_-]//g')
[ $(( `echo "$CN" | wc -m` - 1 )) -lt 64 ] && chck_val=0
done

chck_val=1
while [ $chck_val -eq 1 ] ; do
read -p "Enter email [ admin@my.router ] : " EMAIL
EMAIL=$(echo $EMAIL | sed 's/[^\.@a-zA-Z0-9_-]//g')
[ $(( `echo "$EMAIL" | wc -m` - 1 )) -lt 64 ] && chck_val=0
done

read -p "Enter challenge password [ ] : " CPASS
CPASS=$(echo $CPASS | sed 's/[^\.a-zA-Z0-9_-]//g')

read -p "Enter optional company name [ ] : " ORGN
ORGN=$(echo $ORGN | sed 's/[^\.a-zA-Z0-9_-]//g')

echo -e "\n"


## Check values
[ -z "$COUNTRY" ]  && COUNTRY="`nvram get wl_country_code`"
[ -z "$PROVINCE" ] && PROVINCE="."
[ -z "$CITY" ]     && CITY="."
[ -z "$ORG" ]      && ORG="`nvram get computer_name`"
[ -z "$OU" ]       && OU="."
[ -z "$CN" ]       && CN="$http_ipaddr"
[ -z "$EMAIL" ]    && EMAIL="admin@my.router"
[ -z "$CPASS" ]    && CPASS="."
[ -z "$ORGN" ]     && ORGN="."

echo -e "\v"
echo -e "\033[1m"

cat << EOF

------------------------------------------------

Country			${COUNTRY}
Province		${PROVINCE}
City			${CITY}
Organization		${ORG}
Organization unit	${OU}
Common name (CN)	${CN}
Email address		${EMAIL}
Challenge password	${CPASS}
Optional company name	${ORGN}

------------------------------------------------

EOF

echo -e "\033[0m"
echo -e "\n"
## Ask user if settings are correct
REPLY=""
while [ "$REPLY" != "y" -a "$REPLY" != "yes" -a \
	"$REPLY" != "n" -a "$REPLY" != "no" ]; do
        read -p "Are these settings correct? (yes/no) : " REPLY
	REPLY=`echo $REPLY | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'`
done

[ "$REPLY" == "yes" -o "$REPLY" == "y" ] && GETVAL=0

done

[ -f ca.raw ] || touch ca.raw

cat > ca.raw << EOF
${COUNTRY}
${PROVINCE}
${CITY}
${ORG}
${OU}
${CN}
${EMAIL}
${CPASS}
${ORGN}
EOF

chmod 600 ca.raw
}

do_create_cert()
{
[ -s ca.raw ] || do_get_raw_data
echo -en "\v\033[1m [ Generating HTTPS certificate and private key ."

# create the server private key
openssl genrsa -des3 -passout pass:password -out server.pem 1024 > /dev/null 2>&1

# create the certificate signing request
cat ca.raw | openssl req -config ${ssldir}/openssl.cnf -new -key server.pem -out server.csr -passin pass:password > /dev/null 2>&1

# remove the passphrase from the key
openssl rsa -in server.pem -out server.key -passin pass:password > /dev/null 2>&1

# convert the certificate request into a signed certificate
openssl x509 -req -days 3653 -in server.csr -signkey server.key -out server.crt > /dev/null 2>&1

if [ -f server.crt ]; then
	chmod 644 server.crt
	cp -f server.crt ${dstdir}
fi

if [ -f server.key ]; then
	chmod 600 server.key
	cp -f server.key ${dstdir}
fi

rm -f server.pem server.csr

echo -e ". done ] \033[0m"
}

################################################################################

[ -d ${keydir} ] || mkdir -p -m 700 ${keydir}
[ -d ${dstdir} ] || mkdir -p -m 700 ${dstdir}

rm -rf ${keydir}/*
cd ${keydir}

do_check_cfg
do_create_cert

echo -e "\v"

cd ${indir}

