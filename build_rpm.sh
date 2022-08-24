#!/usr/bin/bash
export PATH=$HOME/SailfishOS/bin:${PATH}
export PWD=`pwd`

# export package_name="harbour-mattermost"
export package_name="ru.sashikknox.wattermost"

# uncomment right engine for your 
# export engine="sfdk engine exec"
export engine="docker exec -u mersdk -w `pwd` sailfish-sdk-build-engine_sashikknox"
# export engine="docker exec --user mersdk -w `pwd` aurora-os-build-engine"

build_dir="build_rpm"
if [[ "${engine}" == *"aurora"* ]]; then
    build_dir="build_aurora_rpm"
fi
version=`grep "Version:" rpm/harbour-mattermost.spec|sed "s/Version: \+\([^ ]\+\)/\1/g"`
release_number=`grep "Release:" rpm/harbour-mattermost.spec|sed "s/Release: \+\([^ ]\+\)/\1/g"`
archive_prefix="${package_name}-${version}"
echo "version=\"${version}\""; 
archive_path="${build_dir}/SOURCES/${archive_prefix}.tar.bz2"
echo "Pack latest git cmmit to an archive: ${archive_path}"
rm -fr `pwd`/${build_dir}/{BUILD,SRPMS}
mkdir -p `pwd`/${build_dir}/SOURCES
git archive --prefix "${archive_prefix}/" --output `pwd`/${archive_path} HEAD


if [[ "${engine}" == *"aurora"* ]]; then
    for each in key cert; do
        if [ -f `pwd`/regular_${each}.pem ]; then 
            echo "Файл ключа regular_${each}.pem найден: OK"
            continue;
        fi
        echo -n "Скачиваем ключ regular_${each}.pem для подписи пактов под АврораОС: "
        curl https://community.omprussia.ru/documentation/files/doc/regular_${each}.pem -o regular_${each}.pem &> /dev/null
        if [ $? -eq 0 ]; then 
            echo "OK"
        else
            echo "FAIL"
            echo "Ошибка скачивания regular_${each}.pem: https://community.omprussia.ru/documentation/files/doc/regular_${each}.pem"
            exit 1
        fi
    done
fi

build_dependencies=""
for each in `grep "BuildRequires:" rpm/harbour-mattermost.spec|sed "s/BuildRequires: \+\([^ ]\+\).*/\1 /g"`; do build_dependencies=" ${build_dependencies} ${each}"; done

# echo "build_dependencies=\"${build_dependencies}\""
# exit

sfdk_targets=`${engine} sb2-config -l|grep -v default|grep aarch`
[ $? -ne 0 ] && echo "FAIL" && exit 1 

echo "WARNING: Build Mattermost Client for ALL your targets in SailfishSDK"
for each in $sfdk_targets; do
    target_arch=${each##*-}
    echo "Build for '$each' target with '$target_arch' architecture"
    # clean build folder
    if [ -d `pwd`/${build_dir}/BUILD ]; then
        rm -fr `pwd`/${build_dir}/BUILD
    fi
    target="${engine} sb2 -t $each"
    #install deps for current target
    ${target} -R -m sdk-install zypper in -y ${build_dependencies}
    
    # build RPM for current target
    ${target} rpmbuild --define "_topdir `pwd`/${build_dir}" --define "_arch $target_arch" --define "_special_name $package_name" -ba `pwd`/rpm/harbour-mattermost.spec
    if [ $? -ne 0 ] ; then 
        echo "Build RPM for ${each} : FAIL"
        continue; 
    fi

    result_rpm_path=`pwd`/${build_dir}/RPMS/${target_arch}/${archive_prefix}-${release_number}.

    # sign RPM packacge 
    if [[ "${engine}" == *"aurora"* ]]; then
        echo -n "Signing RPMs: "
        ${target} rpmsign-external sign --key `pwd`/regular_key.pem --cert `pwd`/regular_cert.pem ${result_rpm_path}*
        if [ $? -ne 0 ] ; then 
            echo "FAIL"
            break; 
        fi
        echo "OK"
        echo -n "Validate RPMs: "
        ${target} rpm-validator -p regular ${result_rpm_path}*
        if [ $? -ne 0 ] ; then 
            echo "FAIL"
            break; 
        fi
        echo "OK"
    elif [[ "${engine}" == "sfdk "* ]] ;then
        echo -n "Validate RPM: "
        sfdk config target=${each}
        sfdk check ${result_rpm_path}*
        if [ $? -ne 0 ] ; then 
            echo "FAIL"
            break;
        fi
        echo "OK"
    fi
done
echo "All build done! All yopur packages in "`pwd`/build_rpm/RPMS
