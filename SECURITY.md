# Security Policy

## Supported Versions

We release patches for security vulnerabilities for the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 0.0.12  | :white_check_mark: |
| < 0.0.12| :x:                |

## Reporting a Vulnerability

We take the security of magickwand seriously. If you discover a security vulnerability, please follow these steps:

### Where to Report

**Please do NOT report security vulnerabilities through public GitHub issues.**

Instead, please report security issues via email to:

- **Email:** [Security contact from repository owner]
- **GitHub Security Advisory:** https://github.com/vnykmshr/magickwand/security/advisories/new

### What to Include

When reporting a vulnerability, please include:

1. **Description**: Clear description of the vulnerability
2. **Impact**: What an attacker could achieve
3. **Steps to Reproduce**: Detailed steps to reproduce the issue
4. **Versions Affected**: Which versions of magickwand are affected
5. **Proposed Fix**: If you have suggestions for fixing the issue
6. **Additional Context**: Any other relevant information

### Response Timeline

- **Initial Response**: Within 48 hours
- **Assessment**: Within 5 business days
- **Fix Timeline**: Depends on severity
  - Critical: Within 7 days
  - High: Within 30 days
  - Medium: Within 90 days
  - Low: Next regular release

### Disclosure Policy

- We ask that you do not publicly disclose the issue until we've had a chance to address it
- Once a fix is released, we will:
  1. Publish a security advisory on GitHub
  2. Credit you in the advisory (unless you prefer to remain anonymous)
  3. Update the CHANGELOG with security fix details

## Security Best Practices

When using magickwand:

1. **Keep Dependencies Updated**: Always use the latest version
   ```bash
   npm update magickwand
   ```

2. **Validate Input**: Always validate user-provided image paths and parameters
   ```javascript
   // Bad - directly using user input
   magickwand.resize(req.query.path, options, callback);

   // Good - validate and sanitize
   const allowedPath = path.join(SAFE_DIR, path.basename(req.query.file));
   if (!fs.existsSync(allowedPath)) return res.status(404).send();
   magickwand.resize(allowedPath, options, callback);
   ```

3. **Resource Limits**: Implement timeouts and size limits to prevent DoS
   ```javascript
   const MAX_SIZE = 4096;
   if (width > MAX_SIZE || height > MAX_SIZE) {
     return res.status(400).send('Image dimensions too large');
   }
   ```

4. **Error Handling**: Never expose internal paths or system information in error messages

5. **ImageMagick Security**: Keep ImageMagick updated on your system
   ```bash
   # macOS
   brew upgrade imagemagick

   # Ubuntu
   sudo apt update && sudo apt upgrade libmagickwand-dev
   ```

## Known Security Considerations

### Image Processing Risks

Image processing libraries can be targets for malicious files:

- **Memory Exhaustion**: Large or malformed images can consume excessive memory
- **Denial of Service**: Processing extremely large images can hang the server
- **File System Access**: Ensure processed images don't write to sensitive locations

### Mitigation Strategies

1. Use input validation
2. Implement resource limits (memory, CPU time)
3. Run in isolated environments (containers, VMs)
4. Monitor and log suspicious activity
5. Keep ImageMagick updated to latest security patches

## Security Updates

Security updates are published through:

1. **GitHub Security Advisories**: https://github.com/vnykmshr/magickwand/security/advisories
2. **npm Advisory Database**: https://www.npmjs.com/advisories
3. **CHANGELOG.md**: All security fixes are documented

To stay informed:
- Watch this repository for security updates
- Enable Dependabot alerts in your projects
- Subscribe to npm security advisories

## Contact

For non-security issues, please use:
- GitHub Issues: https://github.com/vnykmshr/magickwand/issues
- Repository Homepage: https://github.com/vnykmshr/magickwand

Thank you for helping keep magickwand and its users safe!
