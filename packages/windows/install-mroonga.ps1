Param(
  [Parameter(mandatory=$true)][String]$mariadbVersion,
  [Parameter(mandatory=$true)][String]$platform
)

function Wait-UntilRunning($cmdName) {
  do
  {
    $Running = Get-Process $cmdName -ErrorAction SilentlyContinue
    Start-Sleep -m 500
  } while (!$Running)
}

function Wait-UntilTerminate($cmdName) {
  do
  {
    $Running = Get-Process $cmdName -ErrorAction SilentlyContinue
    Start-Sleep -m 500
  } while ($Running)
}

function Install-Mroonga($mariadbVer, $arch, $installSqlDir) {
  Write-Output("Start to install Mroonga")
  cd "mariadb-$mariadbVer-$arch"
  Write-Output("Start mysqld.exe")
  Start-Process .\bin\mysqld.exe
  Start-Sleep -s 10
  Write-Output("Execute install.sql")
  Get-Content "$installSqlDir\install.sql" | .\bin\mysql.exe -uroot
  Write-Output("Shutdown mysqld.exe")
  Start-Process .\bin\mysqladmin.exe -ArgumentList "-uroot shutdown"
  Start-Sleep -s 10
  cd ..
  Write-Output("Finished to install Mroonga")
}

$installSqlDir = ".\share\mroonga"

Install-Mroonga $mariadbVersion $platform $installSqlDir
