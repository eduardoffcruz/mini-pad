# minipad


  -- a simple editor, inspired by [kilo](https://viewsourcecode.org/snaptoken/kilo/) and [nano](https://git.savannah.gnu.org/cgit/nano.git/) --


--------------
### Appearance

In rough ASCII graphics, this is what minipad's screen looks like on your terminal:
<pre>
   _____________________________________________________________________
  |1 This is the text view, displaying the contents of the file you     |
  |2 are editing.                                                       |
  |3                                                                    |
  |4 The first bottom row displays information about the file, like its |
  |5 name, whether it has been modified, current line index and total   |
  |6 number of lines. The last bottom row displays shortcuts to control |
  |7 the editor. The ^ means Ctrl. Sometimes this information is        |
  |8 replaces with some status message or even a a prompt bar when the  |
  |9 editor requires to read some extra input from you.                 |
  |10                                                                   |
   ---------------------------------------------------------------------
  |  filename (modified)                                           3/10 |
   ---------------------------------------------------------------------
  | HELP: ^S = save | ^X = quit | ^F = find    [status message]         |
   _____________________________________________________________________
</pre>
--------------
### Installation
Run the following command to compile and install 'minipad':
```bash
$ ./install.sh 
```
- If you encounter an execution permission issue, use the following command to enable the execution of the 'install.sh' script:
```bash
$ sudo chmod +x install.sh 
```
--------------
### How to use
After installing 'minipad', you can run it from the terminal:
- To edit an existing file or open a new file:
```bash
$ minipad <filename> 
```
- To open a new, untitled instance:
```bash
$ minipad 
```
<!--
### Compile

```bash
$ cd minipad/
$ make
```
  Makefile supports:
    `make`
    `make debug`
    `make clean`
-->
--------------
## Note
A lot of functionality is yet to be implemented (and improved).
- [x] search text
- [x] read & edit files
- [x] save file changes
- [ ] ^N for new file
- [ ] support horizontal scroll bar
- [ ] fix mousepad interaction with terminal (macOS)
- [ ] import editor config from file
- [ ] text highlighing
- [ ] fix `+vogal bug
