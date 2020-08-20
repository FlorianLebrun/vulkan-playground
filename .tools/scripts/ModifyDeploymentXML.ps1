Param (
 [string]$FileName
 ) 
$DeployManifest = [xml](Get-Content $FileName)
#$DeployManifest.assembly.deployment.SetAttribute("mapFileExtensions","true")
$DeployManifest.assembly.deployment.SetAttribute("trustURLParameters","true")
$DeployManifest.save("$FileName")