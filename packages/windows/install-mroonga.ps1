Param(
  [Parameter(mandatory=$true)][String]$mariadbVersion,
  [Parameter(mandatory=$true)][String]$platform
)

function Run-MySQL {
  Write-Output("Start mysqld.exe")
  Start-Process .\bin\mysqld.exe -ArgumentList "--console" -RedirectStandardError ..\mysqld.txt
  $Waiting = $TRUE
  do
  {
    if (Test-Path ..\mysqld.txt) {
      $Version = Get-Content ..\mysqld.txt | Select-String -Pattern "^Version:"
      if ($Version) {
        $Waiting = $FALSE
      }
    }
    Start-Sleep -s 1
  } while ($Waiting)
}

function Run-MySQLInstallDB {
  Write-Output("Start mysql_install_db.exe")
  Start-Process .\bin\mysql_install_db.exe -RedirectStandardOutput ..\install.txt
  $Waiting = $TRUE
  do
  {
    if (Test-Path ..\install.txt) {
      $Successful = Get-Content ..\install.txt | Select-String -Pattern "successful"
      if ($Successful) {
        $Waiting = $FALSE
      }
    }
    Start-Sleep -s 1
  } while ($Waiting)
}

function Run-MySQLAdmin {
  Write-Output("Shutdown mysqld.exe")
  Start-Process .\bin\mysqladmin.exe -ArgumentList "-uroot shutdown"
  $Waiting = $TRUE
  do
  {
    if (Test-Path ..\mysqld.txt) {
      $Complete = Get-Content ..\mysqld.txt | Select-String -Pattern "Shutdown complete"
      if ($Complete) {
        $Waiting = $FALSE
      }
    }
    Start-Sleep -s 1
  } while ($Waiting)
}

function Install-Mroonga($mariadbVer, $arch, $installSqlDir) {
  Write-Output("Start to install Mroonga")
  cd "mariadb-$mariadbVer-$arch"
  if ("$mariadbVer" -eq "10.4.7") {
    Write-Output("Clean data directory")
    Remove-Item data -Recurse
    Run-MySQLInstallDB
  }
  Run-MySQL
  Write-Output("Execute install.sql")
  Get-Content "$installSqlDir\install.sql" | .\bin\mysql.exe -uroot
  Run-MySQLAdmin
  cd ..
  Write-Output("Finished to install Mroonga")
}

$installSqlDir = ".\share\mroonga"

Install-Mroonga $mariadbVersion $platform $installSqlDir
