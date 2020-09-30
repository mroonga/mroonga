Param(
  [Parameter(mandatory=$true)][String]$mariadbVersion,
  [Parameter(mandatory=$true)][String]$platform
)

function Run-MySQL {
  Write-Host "Start mysqld.exe"
  $mysqld = Start-Process `
    .\bin\mysqld.exe `
    -ArgumentList @("--console") `
    -PassThru `
    -NoNewWindow
  while ($TRUE) {
    $mysqladmin = Start-Process `
      .\bin\mysqladmin.exe `
      -ArgumentList @("-uroot", "ping") `
      -PassThru `
      -NoNewWindow `
      -Wait
    if ($mysqladmin.ExitCode -eq 0) {
      Write-Host "Succeeded to run mysqld.exe"
      break;
    }
    if (!($mysqld.ExitCode -eq $null)) {
      Write-Host "Failed to run mysqld.exe"
      exit $mysqld.ExitCode
    }
    Start-Sleep -s 1
  }
  return $mysqld
}

function Run-MySQLInstallDB {
  Write-Host "Start mysql_install_db.exe"
  Start-Process `
    .\bin\mysql_install_db.exe `
    -ArgumentList @("--datadir=data") `
    -NoNewWindow `
    -Wait | Out-Host
}

function Shutdown-MySQL {
  Write-Host "Shutdown mysqld.exe"
  Start-Process `
    .\bin\mysqladmin.exe `
    -ArgumentList @("-uroot", "shutdown") `
    -NoNewWindow `
    -Wait | Out-Host
}

function Install-Mroonga($mariadbVer, $arch, $installSqlDir) {
  Write-Host "Start to install Mroonga"
  cd "mariadb-$mariadbVer-$arch"
  if (Test-Path "data") {
    Write-Host "Clean data directory"
    Remove-Item "data" -Recurse
  }
  if (!(Test-Path "data")) {
    Run-MySQLInstallDB
  }
  $mysqld = Run-MySQL
  Write-Host "Execute install.sql"
  Get-Content "$installSqlDir\install.sql" | .\bin\mysql.exe -uroot
  Shutdown-MySQL
  $mysqld.WaitForExit()
  cd ..
  Write-Host "Finished to install Mroonga"
}

$installSqlDir = ".\share\mroonga"

Install-Mroonga $mariadbVersion $platform $installSqlDir
