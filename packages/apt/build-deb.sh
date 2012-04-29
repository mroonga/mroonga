#!/bin/sh

LANG=C

PACKAGE=$(cat /tmp/build-package)
USER_NAME=$(cat /tmp/build-user)
VERSION=$(cat /tmp/build-version)
DEPENDED_PACKAGES=$(cat /tmp/depended-packages)
BUILD_SCRIPT=/tmp/build-deb-in-chroot.sh

mysql_server_package=mysql-server-5.1

run()
{
    "$@"
    if test $? -ne 0; then
	echo "Failed $@"
	exit 1
    fi
}

grep '^deb ' /etc/apt/sources.list | \
    sed -e 's/^deb /deb-src /' > /etc/apt/sources.list.d/base-source.list

if [ ! -x /usr/bin/aptitude ]; then
    run apt-get update
    run apt-get install -y aptitude
fi
run aptitude update -V -D
run aptitude install -V -D -y --allow-untrusted groonga-keyring
run aptitude safe-upgrade -V -D -y

security_list=/etc/apt/sources.list.d/security.list
if [ ! -d "${security_list}" ]; then
    run aptitude install -V -D -y lsb-release

    distribution=$(lsb_release --id --short)
    code_name=$(lsb_release --codename --short)
    case ${distribution} in
	Debian)
	    if [ "${code_name}" = "sid" ]; then
		touch "${security_list}"
	    else
		cat <<EOF > "${security_list}"
deb http://security.debian.org/ ${code_name}/updates main
deb-src http://security.debian.org/ ${code_name}/updates main
EOF
	    fi
	    ;;
	Ubuntu)
	    cat <<EOF > "${security_list}"
deb http://security.ubuntu.com/ubuntu ${code_name}-security main restricted
deb-src http://security.ubuntu.com/ubuntu ${code_name}-security main restricted
EOF
	    ;;
    esac

    run aptitude update -V -D
    run aptitude safe-upgrade -V -D -y
fi

run aptitude install -V -D -y devscripts ${DEPENDED_PACKAGES}
run aptitude build-dep -V -D -y ${mysql_server_package}
run aptitude clean

if ! id $USER_NAME >/dev/null 2>&1; then
    run useradd -m $USER_NAME
fi

cat <<EOF > $BUILD_SCRIPT
#!/bin/sh

rm -rf build
mkdir -p build

cp /tmp/${PACKAGE}-${VERSION}.tar.gz build/${PACKAGE}_${VERSION}.orig.tar.gz

rm -rf mysql-package
mkdir -p mysql-package
cd mysql-package
apt-get source ${mysql_server_package}
cd */debian/..
debuild -us -uc -Tconfigure
make -C builddir/include
make -C builddir/scripts
cd ../..

cd build
ln -fs \$(find ../mysql-package -maxdepth 1 -type d | tail -1) mysql

tar xfz ${PACKAGE}_${VERSION}.orig.tar.gz
cd ${PACKAGE}-${VERSION}/
cp -rp /tmp/${PACKAGE}-debian debian
# export DEB_BUILD_OPTIONS="noopt nostrip"
debuild -us -uc
EOF

run chmod +x $BUILD_SCRIPT
run su - $USER_NAME $BUILD_SCRIPT
