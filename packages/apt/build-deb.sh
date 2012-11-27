#!/bin/sh

LANG=C

PACKAGE=$(cat /tmp/build-package)
USER_NAME=$(cat /tmp/build-user)
VERSION=$(cat /tmp/build-version)
DEPENDED_PACKAGES=$(cat /tmp/depended-packages)
BUILD_SCRIPT=/tmp/build-deb-in-chroot.sh

mysql_server_package=mysql-server

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

run apt-get update -V
run apt-get install -V -y lsb-release
distribution=$(lsb_release --id --short)
code_name=$(lsb_release --codename --short)

groonga_list=/etc/apt/sources.list.d/groonga.list
if [ ! -f "${groonga_list}" ]; then
    case ${distribution} in
	Debian)
	    component=main
	    ;;
	Ubuntu)
	    component=universe
	    ;;
    esac
    downcased_distribtion=$(echo ${distribution} | tr A-Z a-z)
    run cat <<EOF | run tee ${groonga_list}
deb http://packages.groonga.org/${downcased_distribtion}/ ${code_name} ${component}
deb-src http://packages.groonga.org/${downcased_distribtion}/ ${code_name} ${component}
EOF
    apt-get update -V
    run apt-get -V -y --allow-unauthenticated install groonga-keyring
fi

run apt-get update -V
run apt-get upgrade -V -y

security_list=/etc/apt/sources.list.d/security.list
if [ ! -f "${security_list}" ]; then
    run apt-get install -V -y lsb-release

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

    run apt-get update -V
    run apt-get upgrade -V -y
fi

universe_list=/etc/apt/sources.list.d/universe.list
if [ ! -f "$universe_list}" ]; then
    case ${distribution} in
	Ubuntu)
	    sed -e 's/main/universe/' /etc/apt/sources.list > ${universe_list}
	    run apt-get update -V
	    ;;
    esac
fi

run apt-get install -V -y devscripts ${DEPENDED_PACKAGES}
run apt-get build-dep -V -y ${mysql_server_package}
run apt-get clean

if ! id $USER_NAME >/dev/null 2>&1; then
    run useradd -m $USER_NAME
fi

cat <<EOF > $BUILD_SCRIPT
#!/bin/sh

rm -rf build
mkdir -p build

cp /tmp/${PACKAGE}-${VERSION}.tar.gz build/${PACKAGE}_${VERSION}.orig.tar.gz

mkdir -p mysql-package
cd mysql-package
apt-get source ${mysql_server_package}
rm -f mysql
MYSQL_SOURCE_DIR=\$(find . -maxdepth 1 -type d | sort | tail -1)
ln -s \$MYSQL_SOURCE_DIR mysql
cd */debian/..
debuild -us -uc -Tconfigure
make -C builddir/include
make -C builddir/scripts
cd ../..

cd build
ln -fs ../mysql-package/mysql ./

tar xfz ${PACKAGE}_${VERSION}.orig.tar.gz
cd ${PACKAGE}-${VERSION}/
cp -rp /tmp/${PACKAGE}-debian debian
# export DEB_BUILD_OPTIONS="noopt nostrip"
case \$MYSQL_SOURCE_DIR in
  *mysql-5.1*)
  MYSQL_SOURCE_VERSION=\${MYSQL_SOURCE_DIR##./mysql-5.1-}
  sed -i "s/@VERSION@/\$MYSQL_SOURCE_VERSION/" debian/control
  ;;
  *mysql-5.5*)
  MYSQL_SOURCE_VERSION=\${MYSQL_SOURCE_DIR##./mysql-5.5-}
  sed -i "s/@VERSION@/\$MYSQL_SOURCE_VERSION/" debian/control
  ;;
esac
debuild -us -uc
EOF

run chmod +x $BUILD_SCRIPT
run su - $USER_NAME $BUILD_SCRIPT
