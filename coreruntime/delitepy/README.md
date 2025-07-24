<!-- DL_DOCS::WEBSITE_BLOCK_START
```{eval-rst}
:orphan:
```
DL_DOCS::WEBSITE_BLOCK_END -->

# DelitePy

## Documentation

### Installing dependencies (for building documentation)

1. Activate your Python 3 virtual environment.
2. `python3 -m pip install -r coreruntime/delitepy/requirements.txt`

### Building documentation

1. `cd "$(git rev-parse --show-toplevel)/coreruntime"`
2. `python3 build.py --doc`\
   or, `delitepy/scripts/build_docs.sh build/delitepy`

The built documentation can be found here: `coreruntime/build/delitepy/docs/html/index.html`.
