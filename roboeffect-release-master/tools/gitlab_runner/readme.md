## How to build project in github Codespaces:

1. goto https://github.com/codespaces, create a New Codespaces.
2. use https://github.com/castleodinland/roboeffect-demo-release for Repository.
3. after vscode ready, run

```bash
docker pull castle6610/andes_build_tools_v323:latest
```

4. run :
```bash
docker run -it --rm \
  -v /workspaces/roboeffect-demo-release:/workspace \
  -w /workspace \
  castle6610/andes_build_tools_v323:latest \
  /bin/bash
```
to enter the container, and current path should be
```bash
/workspace
```

5. run
```bash
chmod -x build.sh
```   

6. run 
```bash
./build.sh build
```

## one script to run building in github codespaces
copy run_codespaces.sh to root of project and run "run_codespaces.sh"