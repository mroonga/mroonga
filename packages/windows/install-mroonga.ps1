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
  cd "mariadb-$mariadbVer-$arch"
  Start-Process .\bin\mysqld.exe
  Wait-UntilRunning mysqld
  Get-Content "$installSqlDir\install.sql" | .\bin\mysql.exe -uroot
  Start-Process .\bin\mysqladmin.exe -ArgumentList "-uroot shutdown"
  Wait-UntilTerminate mysqld
  cd ..
}

$installSqlDir = ".\share\mroonga"

Install-Mroonga $mariadbVersion $platform $installSqlDir
