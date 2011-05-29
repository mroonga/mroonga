#!/bin/sh

LANG=C

PACKAGE=$(cat /tmp/build-package)
USER_NAME=$(cat /tmp/build-user)
VERSION=$(cat /tmp/build-version)
DEPENDED_PACKAGES=$(cat /tmp/depended-packages)
BUILD_SCRIPT=/tmp/build-${PACKAGE}.sh

run()
{
    "$@"
    if test $? -ne 0; then
	echo "Failed $@"
	exit 1
    fi
}

distribution=$(cut -d ' ' -f 1 /etc/redhat-release | tr 'A-Z' 'a-z')
distribution_version=$(cut -d ' ' -f 3 /etc/redhat-release)
if ! rpm -q ${distribution}-release > /dev/null 2>&1; then
    packages_dir=/var/cache/yum/core/packages
    release_rpm=${distribution}-release-${distribution_version}-*.rpm
    run rpm -Uvh --force ${packages_dir}/${release_rpm}
    run rpm -Uvh --force ${packages_dir}/ca-certificates-*.rpm
fi

if ! rpm -q groonga-repository > /dev/null 2>&1; then
    run rpm -Uvh http://packages.groonga.org/${distribution}/groonga-repository-1.0.0-0.noarch.rpm
fi

case $distribution in
    fedora)
	DEPENDED_PACKAGES="$DEPENDED_PACKAGES mysql-devel"
	DEPENDED_PACKAGES="$DEPENDED_PACKAGES cmake libaio-devel systemtap-sdt-devel"
	;;
    centos)
	;;
esac

run yum update -y
run yum install -y rpm-build tar ${DEPENDED_PACKAGES}
run yum clean packages

if ! id $USER_NAME >/dev/null 2>&1; then
    run useradd -m $USER_NAME
fi

cat <<EOF > $BUILD_SCRIPT
#!/bin/sh

if [ ! -f ~/.rpmmacros ]; then
    cat <<EOM > ~/.rpmmacros
%_topdir \$HOME/rpm
EOM
fi

# rm -rf rpm
mkdir -p rpm/SOURCES
mkdir -p rpm/SPECS
mkdir -p rpm/BUILD
mkdir -p rpm/RPMS
mkdir -p rpm/SRPMS

cp /tmp/groonga-storage-engine-$VERSION.tar.gz rpm/SOURCES/
cp /tmp/${PACKAGE}.spec rpm/SPECS/

chmod o+rx . rpm rpm/RPMS rpm/SRPMS

rpmbuild -ba rpm/SPECS/${PACKAGE}.spec
EOF

run chmod +x $BUILD_SCRIPT
run su - $USER_NAME $BUILD_SCRIPT
