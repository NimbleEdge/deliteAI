# DeliteAI documentation

For the online documentation, please visit [deliteai.dev](https://deliteai.dev/).

## Build documentation

### Set up the prerequisites

1. Activate your Python 3 virtual environment.
2. `cd "$(git rev-parse --show-toplevel)/docs"`
3. `python3 -m pip install -r requirements.txt`

### Build HTML documentation

1. Activate your Python 3 virtual environment.
2. `cd "$(git rev-parse --show-toplevel)/docs"`
3. `./scripts/run build_website`

The built documentation can be found here: [`docs/build/deliteai.dev/html/index.html`](./build/deliteai.dev/html/index.html).
