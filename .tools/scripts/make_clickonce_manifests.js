const { script, command, file } = require("./cli")
const Process = require("process")
const Path = require("path")

// Manifest Generator tool
const mageExe = Path.resolve("./signtool/mage.exe")

// Certificate infos
let certificateFile, certificatePassword
if (Process.env["WYDE-BUILD-CERTIFICATE-FILE"]) {
  certificateFile = Process.env["WYDE-BUILD-CERTIFICATE-FILE"]
  certificatePassword = Process.env["WYDE-BUILD-CERTIFICATE-PASSWORD"]
}
else {
  certificateFile = Path.resolve("./signtool/default-certificate.pfx")
  certificatePassword = "anyone_password"
}

script((argv) => {
  console.log(`Make clickonce manifests for '${argv.name}' application at '${argv.destination}'`)
  const applicationDir = Path.resolve(argv.destination)
  const applicationVersion = argv.version + ".0"

  const applicationManifest = Path.join(applicationDir, argv.name + ".manifest")
  const deploymentManifest = Path.join(applicationDir, argv.name + ".application")

  // Clean previous manifest
  file.remove(applicationManifest)
  file.remove(deploymentManifest)

  // Generate and sign application manifest
  command.call(mageExe, [
    "-New", "Application",
    "-Processor", "amd64",
    "-ToFile", applicationManifest,
    "-FromDirectory", applicationDir,
    "-Name", "COWydeWebNode",
    "-Version", applicationVersion,
    "-Algorithm", "sha256RSA",
  ])
  command.call(mageExe, [
    "-Sign", applicationManifest,
    "-CertFile", certificateFile,
    "-Password", certificatePassword,
  ])

  // Generate and sign deployment manifest
  command.call(mageExe, [
    "-New", "Deployment",
    "-Processor", "amd64",
    "-Install", "false",
    "-Publisher", "Mphasis Wyde",
    "-AppManifest", applicationManifest,
    "-ToFile", deploymentManifest,
    "-Version", applicationVersion,
    "-Algorithm", "sha256RSA",
  ])
  command.call("powershell", [
    "-file", "ModifyDeploymentXML.ps1",
    deploymentManifest,
  ], { cwd: __dirname })
  command.call(mageExe, [
    "-Sign", deploymentManifest,
    "-CertFile", certificateFile,
    "-Password", certificatePassword,
  ])
}, {
  arguments: {
    "name": {
      type: "string",
      required: true,
    },
    "version": {
      type: "string",
      required: true,
    },
    "destination": {
      type: "string",
      required: true,
    }
  }
})


