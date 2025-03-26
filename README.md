# Hoax
<strong>Hoax</strong> is an alternative to <strong>CheatEngine</strong> for Linux users. It aims to provide a more complete feature set than other alternatives such as PINCE and GameConqueror <br><br>

![hoax.png](https://i.postimg.cc/BQj35mm6/image.png)

## Requirements
`libscanmem`&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp; scanner for Linux<br>
`gdb` &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp; &ensp;allows for process debugging<br>
`fltk`&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp; &ensp;  C++ UI library<br>
`python3`&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp; &ensp; for scripts<br>
`python3-pygments`&ensp; assembly syntax highlighting<br>

## Issues
- [ ] <strong>TODO</strong> process list does not update <strong>good first commit!</strong>
- [ ] <strong>TODO</strong> documentation or guide <strong>good first commit!</strong>
- [ ] <strong>TODO</strong> address value updater (background thread)

## Roadmap
- [ ] documentation <strong>good first commit!</strong>
- [ ] theming <strong>good first commit!</strong>
- [ ] standardized configuration <strong>good first commit!</strong>
- [ ] plugin protocol
- [ ] signature generator
- [ ] assembler (replace existing assembly with your own) <i>[CE feature]</i>
- [x] find out what accesses / writes to this address <i>[CE feature]</i>
- [x] memory view
- [ ] scanning
     - [x] strings
     - [x] byte arrays
     - [x] ints/floats
     - [ ] comprehensive expressions
- [ ] pointer scan <i>[CE feature]</i>
- [x] save feature <i>[CE feature]</i>


## Contribute
Contributions are highly welcomed, as Hoax strives to become a feature complete CheatEngine alternative in the future.

Make sure that changes are in line with the project direction, utilize the roadmap found in README.md or open an issue if unsure.

**Recommendation: For your first couple of PRs, start with something small to get familiar with the project and its development processes.**

## Guidelines
* Match casing with existing code
* Both C and C++ code is welcomed
* Python is the primary scripting language for interfacing with GDB and scanmem

## Example Git workflow
The recommended way to work on Torr is by cloning the main repository locally, then forking it on GitHub and adding your repository as a Git remote:
```sh
git remote add myfork git@github.com:MyUsername/Hoax.git
```

You can then create a new branch and start making changes to the code:
```sh
git checkout -b mybranch
```

And finally push the commits to your fork:
```sh
# ONLY run this the first time you push
git push --set-upstream myfork mybranch
# This will work for further pushes
git push
```

If you wish to sync your branch with master, or locally resolve merge conflicts, use:
```sh
# On mybranch
git fetch origin
git rebase master
```

## Communication

GitHub is the main platform for communication @ DevGev/Hoax.
