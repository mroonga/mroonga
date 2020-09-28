Param(
  [Parameter(mandatory=$true)][String]$mariadbVersion,
  [Parameter(mandatory=$true)][String]$platform
)

function Run-MySQL($logPath) {
  Write-Output("Start mysqld.exe")
  $mysqld = Start-Process ^
    .\bin\mysqld.exe ^
    -ArgumentList ^
    "--console" ^
    -RedirectStandardError $logPath ^
    -PassThru
  while ($TRUE) {
    $version = Get-Content $logPath | Select-String -Pattern "^Version:"
    if ($version) {
      break;
    }
    if (!($mysqld.ExitCode -eq $null)) {
      Write-Output("Failed to run mysqld.exe")
      Get-Content $logPath
      exit $mysqld.ExitCode
    }
    Start-Sleep -s 1
  }
  return $mysqld
}

function Run-MySQLInstallDB {
  Write-Output("Start mysql_install_db.exe")
  Start-Process .\bin\mysql_install_db.exe -Wait
}

function Shutdown-MySQL {
  Write-Output("Shutdown mysqld.exe")
  Start-Process .\bin\mysqladmin.exe -ArgumentList "-uroot shutdown" -Wait
}

function Install-Mroonga($mariadbVer, $arch, $installSqlDir) {
  Write-Output("Start to install Mroonga")
  cd "mariadb-$mariadbVer-$arch"
  if (Test-Path "data") {
    Write-Output("Clean data directory")
    Remove-Item "data" -Recurse
  }
  if (!(Test-Path "data")) {
    Run-MySQLInstallDB
  }
  $mysqlLogPath = "..\mysqld.log"
  New-Item $mysqlLogPath -ItemType File
  $mysqld = Run-MySQL $mysqlLogPath
  Write-Output "Execute install.sql"
  Get-Content "$installSqlDir\install.sql" | .\bin\mysql.exe -uroot
  Shutdown-MySQL
  $mysqld | Wait-Proces
  Remove-Item $mysqlLogPath
  cd ..
  Write-Output("Finished to install Mroonga")
}

$installSqlDir = ".\share\mroonga"

Install-Mroonga $mariadbVersion $platform $installSqlDir
