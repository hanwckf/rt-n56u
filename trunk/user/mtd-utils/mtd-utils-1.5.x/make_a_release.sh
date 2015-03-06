#!/bin/sh -uef

# A small helper script to release mtd-utils. Takes the new version
# as a parameter.

fatal() {
        printf "Error: %s\n" "$1" >&2
        exit 1
}

usage() {
        cat <<EOF
Usage: ${0##*/} <new_ver> <outdir>

<new_ver>  - mtd utils version to create in X.Y.Z format
<outdir>   - the output directory where to store the tarball with the
             gpg signature
EOF
        exit 0
}

[ $# -eq 0 ] && usage
[ $# -eq 2 ] || fatal "Insufficient or too many argumetns"

new_ver="$1"; shift
outdir="$1"; shift

release_name="mtd-utils-$new_ver"
tag_name="v$new_ver"

# Make sure the input is sane and the makefile contains sensible version
echo "$new_ver" | egrep -q -x '[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+' ||
        fatal "please, provide new version in X.Y.Z format"

egrep -q -x 'VERSION = [[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+' Makefile ||
        fatal "Makefile does not contain \"Version = X.Y.Z\" line"

# Make sure the git index is up-to-date
[ -z "$(git status --porcelain)" ] || fatal "Git index is not up-to-date"

# Make sure the tag does not exist
[ -z "$(git tag -l "$tag_name")" ] || fatal "Tag $tag_name already exists"

# Change the version in the Makefile
sed -i -e "s/^VERSION = [[:digit:]]\+\.[[:digit:]]\+\.[[:digit:]]\+/VERSION = $new_ver/" Makefile

# And commit the change
git commit -s -m "Release $release_name" Makefile

# Create new signed tag
echo "Signing tag $tag_name"
git tag -m "$release_name" -s "$tag_name"

# Prepare signed tarball
git archive --format=tar --prefix="$release_name/" "$tag_name" | \
        bzip2 > "$outdir/$release_name.tar.bz2"
echo "Signing the tarball"
gpg -o "$outdir/$release_name.tar.bz2.asc" --detach-sign -a "$outdir/$release_name.tar.bz2"

scp_url="casper.infradead.org:/var/ftp/pub/mtd-utils"
ftp_url="ftp://ftp.infradead.org/pub/mtd-utils"
git_url="git://git.infradead.org/mtd-utils.git"

cat <<EOF1
Created $outdir/$release_name.tar.bz2
Please, verify, then push the tag and upload the tarball and the signature
You can use these commands:

------------------------------------------------------------------------------
git push origin master $tag_name
scp $outdir/$release_name.tar.bz2 $outdir/$release_name.tar.bz2.asc $scp_url
------------------------------------------------------------------------------

Please, send an announcement, below is the command you may run in your
run. Substitute "me" with your e-mail address if needed, although it is
cleaner to configure 'git send-email' to interpret 'me' as an alias for
your name/email, see 'sendemail.aliasesfile' git configuration option.

------------------------------------------------------------------------------
mtd_tmpfile=\$(mktemp)

cat > \$mtd_tmpfile <<EOF
Subject: [ANNOUNCE] $release_name is released

Hi,

$release_name is released.

Tarball:               $ftp_url/$release_name.tar.bz2
Tarball gpg signature: $ftp_url/$release_name.tar.bz2.asc
Signed git tag:        $git_url $tag_name
EOF

git send-email --from me --to 'MTL Mailing List <linux-mtd@lists.infradead.org>' --cc 'Peter Korsgaard (buildroot) <jacmet@sunsite.dk>' --cc 'Josh Boyer (Fedora) <jwboyer@gmail.com>' --cc 'Riku Voipio (Debian) <riku.voipio@linaro.org>' \$mtd_tmpfile

rm \$mtd_tmpfile
------------------------------------------------------------------------------
EOF1
