
$cert = New-SelfSignedCertificate -DnsName "anyone" -Type CodeSigning -CertStoreLocation Cert:\CurrentUser\My
$CertPassword = ConvertTo-SecureString -String "anyone_password" -Force -AsPlainText
Export-PfxCertificate -Cert "cert:\CurrentUser\My\$($cert.Thumbprint)" -FilePath "$($PSScriptRoot)\default-certificate.pfx" -Password $CertPassword
