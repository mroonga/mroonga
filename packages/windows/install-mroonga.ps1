Param(
  [Parameter(mandatory=$true)][String]$mariadbVersion,
  [Parameter(mandatory=$true)][String]$platform
)

function Run-MySQL($LogPath) {
  Write-Output("Start mysqld.exe")
  Start-Process .\bin\mysqld.exe -ArgumentList "--console" -RedirectStandardError $logPath
  while ($TRUE) {
    $version = Get-Content $LogPath | Select-String -Pattern "^Version:"
    if ($version) {
      break;
    }
    Start-Sleep -s 1
  }
}

function Run-MySQLInstallDB {
  Write-Output("Start mysql_install_db.exe")
  $logPath = "..\install.log"
  New-Item $logPath -ItemType File
  Start-Process .\bin\mysql_install_db.exe -RedirectStandardOutput $logPath
  while ($TRUE) {
    $successful = Get-Content $logPath | Select-String -Pattern "successful"
    if ($successful) {
      break;
    }
    Start-Sleep -s 1
  }
  Remove-Item $logPath -Force
}

function Shutdown-MySQL($LogPath) {
  Write-Output("Shutdown mysqld.exe")
  Start-Process .\bin\mysqladmin.exe -ArgumentList "-uroot shutdown"
  while ($TRUE) {
    $complete = Get-Content $LogPath | Select-String -Pattern "Shutdown complete"
    if ($complete) {
      break;
    }
    Start-Sleep -s 1
  }
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
  Run-MySQL $mysqlLogPath
  Write-Output "Execute install.sql"
  Get-Content "$installSqlDir\install.sql" | .\bin\mysql.exe -uroot
  Shutdown-MySQL $mysqlLogPath
  Remove-Item $mysqlLogPath
  cd ..
  Write-Output("Finished to install Mroonga")
}

$installSqlDir = ".\share\mroonga"

Install-Mroonga $mariadbVersion $platform $installSqlDir
