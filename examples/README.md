# MagickWand Examples

This directory contains usage examples for the MagickWand library.

## CDN Middleware (cdn.js)

Express/Connect middleware for dynamic image resizing, suitable for building a CDN-style image service.

### Features

- On-demand image resizing via URL parameters
- Configurable URL patterns (e.g., `/cache/300x200/photo.jpg`)
- Optional whitelist of allowed dimensions
- File type validation (jpg, gif, png)
- Automatic content-type detection

### Installation

Install the required dependencies:

```bash
npm install express mime
```

### Basic Usage

```javascript
const express = require('express');
const cdnCache = require('magickwand/examples/cdn');

const app = express();

// Configure CDN middleware
app.use(cdnCache({
  path: 'cache',                    // URL prefix: /cache/...
  srcPath: 'public/images/',        // Source images directory
  fileTypes: ['.jpg', '.png'],      // Allowed file extensions
  validSizes: ['300x200', '600x400'] // Optional: whitelist dimensions
}));

app.listen(3000, () => {
  console.log('CDN server running on http://localhost:3000');
});
```

### URL Pattern

```
http://localhost:3000/cache/{width}x{height}/{filename}
```

**Example:**
```
http://localhost:3000/cache/300x200/photo.jpg
```

This will:
1. Load `public/images/photo.jpg`
2. Resize to 300x200 pixels
3. Return resized image with proper content-type header

### Configuration Options

```javascript
cdnCache({
  // URL path prefix (default: 'cache')
  path: 'cache',

  // Source images directory (default: 'public/images/')
  srcPath: 'public/images/',

  // Allowed file extensions (default: ['.jpg', '.gif', '.png'])
  fileTypes: ['.jpg', '.png', '.gif'],

  // MIME type pattern to match (default: '^image/')
  mime: '^image/',

  // Optional: whitelist of allowed dimensions
  // If not set, any dimensions are allowed
  validSizes: ['100x100', '300x200', '600x400', '1200x800'],

  // Optional: custom parameter parser
  // Override to implement different URL patterns
  getParams: (url) => {
    // Return: { width, height, path } or null
  }
})
```

### Complete Example

```javascript
const express = require('express');
const path = require('path');
const cdnCache = require('magickwand/examples/cdn');

const app = express();
const PORT = 3000;

// Serve original images
app.use('/images', express.static('public/images'));

// Dynamic image resizing
app.use(cdnCache({
  path: 'thumbs',
  srcPath: path.join(__dirname, 'public/images/'),
  fileTypes: ['.jpg', '.jpeg', '.png', '.gif'],
  validSizes: [
    '150x150',   // Thumbnail
    '300x300',   // Small
    '600x600',   // Medium
    '1200x1200'  // Large
  ]
}));

// Error handling
app.use((req, res) => {
  res.status(404).send('Not Found');
});

app.listen(PORT, () => {
  console.log(`Image CDN running on http://localhost:${PORT}`);
  console.log('Examples:');
  console.log(`  http://localhost:${PORT}/images/photo.jpg (original)`);
  console.log(`  http://localhost:${PORT}/thumbs/150x150/photo.jpg (thumbnail)`);
});
```

### Security Considerations

1. **Whitelist dimensions**: Use `validSizes` to prevent resource exhaustion from arbitrary resize requests
2. **File type validation**: Restrict `fileTypes` to prevent processing of non-image files
3. **Source path restriction**: Ensure `srcPath` points to a dedicated images directory, not system root
4. **Rate limiting**: Consider adding rate limiting middleware for production use
5. **Caching**: Add HTTP caching headers or use a CDN in front for production

### Production Recommendations

For production use, consider:

```javascript
const rateLimit = require('express-rate-limit');
const apicache = require('apicache');

// Rate limiting
app.use('/thumbs', rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 100 // limit each IP to 100 requests per windowMs
}));

// Memory caching
const cache = apicache.middleware;
app.use(cache('5 minutes'));

// CDN middleware
app.use(cdnCache({
  path: 'thumbs',
  srcPath: process.env.IMAGES_PATH,
  validSizes: ['150x150', '300x300', '600x600']
}));
```

### Error Handling

The middleware returns 404 for:
- Invalid URL patterns
- Non-existent source images
- Invalid file types
- Dimensions not in `validSizes` (if configured)
- Image processing errors

### Performance Notes

- Resizing happens on-demand (not pre-generated)
- Consider adding a caching layer (Redis, filesystem) for frequently accessed sizes
- Use a reverse proxy (nginx, CloudFront) for serving static resized images
- Monitor server resources under load

## Adding More Examples

To contribute additional examples:

1. Create a new `.js` file in this directory
2. Document it in this README
3. Follow the coding standards (ES6+, see CONTRIBUTING.md)
4. Ensure it passes linting: `npm run lint`

## License

MIT (same as main project)
