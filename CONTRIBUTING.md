# Contributing to MagickWand

Thank you for your interest in contributing to MagickWand! This document provides guidelines for contributing to the project.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/magickwand.git
   cd magickwand
   ```
3. **Install dependencies**:
   ```bash
   npm install
   ```
4. **Build the native module**:
   ```bash
   npm run install
   ```

## Development Workflow

### Making Changes

1. **Create a feature branch**:
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes** following the coding standards below

3. **Run tests** to ensure nothing breaks:
   ```bash
   npm test
   ```

4. **Run linter** to check code quality:
   ```bash
   npm run lint
   ```

5. **Commit your changes** with clear, descriptive messages:
   ```bash
   git commit -m "Add feature: description of what you did"
   ```

### Coding Standards

#### JavaScript
- Use ES6+ syntax where appropriate
- Follow the ESLint configuration (`.eslintrc.json`)
- Use 2 spaces for indentation
- Use single quotes for strings
- Add JSDoc comments for all public functions

#### C/C++
- Follow existing code style
- Keep changes minimal and focused
- Test on both macOS and Linux if possible
- Ensure compatibility with both ImageMagick 6 and 7

### Testing

- All new features must include tests
- Tests are located in `test/test.js`
- Use Mocha's `describe` and `it` blocks
- Aim for clear, descriptive test names

Example:
```javascript
it('should resize image maintaining aspect ratio when height is 0', function(done) {
  magickwand.resize(imagePath, { width: 100, height: 0 }, function(err, data, info) {
    assert(err === undefined);
    assert(info.width === 100);
    assert(info.height > 0);
    done();
  });
});
```

## Pull Request Process

1. **Update documentation** if you're changing functionality
2. **Update CHANGELOG.md** with your changes
3. **Ensure all tests pass** (`npm test`)
4. **Ensure code quality** (`npm run lint`)
5. **Create a pull request** with a clear title and description:
   - What does this PR do?
   - Why is this change necessary?
   - How has it been tested?

### Pull Request Title Format

Use conventional commit format:
- `feat: Add new feature`
- `fix: Fix bug description`
- `docs: Update documentation`
- `test: Add tests for X`
- `refactor: Refactor code`
- `chore: Update dependencies`

## Reporting Issues

### Bug Reports

When reporting bugs, please include:
- Node.js version (`node --version`)
- ImageMagick version (`magick --version` or `convert --version`)
- Operating system and version
- Minimal code to reproduce the issue
- Error messages and stack traces
- Expected vs actual behavior

### Feature Requests

When requesting features:
- Explain the use case
- Describe the desired behavior
- Provide examples if possible
- Consider whether it fits the project's scope

## Code of Conduct

### Our Standards

- Be respectful and inclusive
- Welcome newcomers
- Focus on constructive feedback
- Accept criticism gracefully
- Prioritize the project's best interests

### Unacceptable Behavior

- Harassment or discrimination
- Trolling or insulting comments
- Publishing others' private information
- Other unprofessional conduct

## Development Tips

### Building on macOS

If you encounter issues with OpenMP:
```bash
brew install imagemagick --without-openmp
```

### Debugging Native Code

To build in debug mode:
```bash
node-gyp configure --debug
node-gyp build
```

### Testing Against Different ImageMagick Versions

Test against both ImageMagick 6 and 7 if possible:
```bash
# ImageMagick 6
brew install imagemagick@6
brew link imagemagick@6 --force

# ImageMagick 7
brew unlink imagemagick@6
brew install imagemagick
```

## Questions?

If you have questions that aren't covered here:
- Check existing issues and pull requests
- Open a new issue with the `question` label
- Be specific about what you're trying to do

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
