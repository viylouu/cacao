# cacao
cacao is a minimal game engine written in c99 with basically *no* dependencies  

## license
this project is licensed under the LGPLv3 copyleft license  
more info in [LICENSE.LGPL](./LICENSE.LGPL)  

## usage
this project can be easily used, by doing a few things  
this process should probably be automated in the future, since its not repo specific.  

heres the process (yes i use the ssh link, i dont care if you like https better, shut up):  
- make your project
- if you're using git/github:
  - run `git submodule add git@github.com:viylouu/cacao.git`
- if you're not using git/github:
  - clone the repo (`git clone git@github.com:viylouu/cacao.git`)
  - copy the contents of the repo into a folder in your project called "cacao"
- then you can copy data to your repo's root directory
- if you want this to be easier to use, you can do a few things:
  - copy the build.sh into the project root
  - if you're using github:
    - you can make an update script using these contents
    - `cd cacao && git checkout -- . && cd .. && git submodule update --remote --recursive --init && rm -rf data/eng && cp -r cacao/data/eng data/eng`
    - then (if you're on linux) you can `chmod +x update.sh`
    - when cacao engine updates, you can now just run `./update.sh` and it will auto update everything
  - if you're not using github:
    - updating will mean updating the cloned repo
    - you can run `git reset --hard && git pull` to update
    - and then copy the contents below
  - copy the `build.sh` into the project root
  - copy the `compile_flags.txt` into the project root (this is for neovim to have proper errors)
  - copy the `data` folder into the project root (REQUIRED)
  - copy the `.gitignore` into the project root
  - copy the `.gitattributes` into the project root
