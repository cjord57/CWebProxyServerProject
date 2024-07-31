This zipfile contains the assignments for Concepts of Programming Languages.  It includes

* `play` - A folder for your scratch files and experiments
* `src/main/scala` - A folder for homework 
* `src/test/scala` - A folder for homework test files - _Do not edit_
* `build.sbt` - A build file - _Do not edit_

`sbt` will create additional folders (`project` and `target`).  If you delete
these, `sbt` will recreate them.

`VSCode` creates _hidden_ folders (`.bloop`, `.bsp`, `.metals`,
`.scalafmt.conf`, `.vscode`).  If you delete these, `VSCode` will recreate
them.

# Software Requirements

You will need to install:

* [Java 17 JDK](https://docs.aws.amazon.com/corretto/latest/corretto-17-ug/downloads-list.html)
  - On MacOs, use the `pkg`
  - On Linux, use `deb` or `rpm`, as appropriate
  - On Windows, use the `msi`
* [`sbt` (formerly known as the Scala Build Tool)](https://www.scala-sbt.org/download.html)
  - On MacOs, use [homebrew](https://brew.sh) - alternative is [sdkman](https://sdkman.io)
  - On Linux, use `deb` or `rpm`, as appropriate
  - On Windows, use the `msi` - alternatives are [scoop](https://scoop.sh) and [sdkman](https://sdkman.io)
  - On Windows, you may need to reboot after install - this ensures that `sbt` is in your `PATH`
* [Visual Studio Code (VSCode)](https://code.visualstudio.com/)

# Using VSCode

Set up Scala in VSCode:

* Install the Scala (Metals) Extension
  - Select `View -> Extension` 
  - Search `Metals`
  - Install `Scala (Metals)`
* Open a workspace for this class
  - Select `File -> Open Folder`
  - Choose the `hw` Folder, which contains this file
  - Make sure to choose the Folder, not any file inside of it
  - VSCode should detect `sbt`
  - When prompted, select `Import Build` (This may happen several times)
  - If you get an error, restart VSCode -- You shouldn't have to repeat this process
* Get formatting working
  - Open a `.scala` file in the editor
  - Select `View -> Command Palette...`
  - In the Palette, Type `Format Document`
  - VSCode should prompt you to create a format
  - When prompted, select `Create .scalafmt.conf`
  - In the Palette, try `Format Document` again
  - If it does not work, restart VSCode -- I had to do this on one machine

# `sbt`

`sbt` is used to compile and test the Scala assignments.  `sbt` will take
care of downloading the correct version of Scala for you.  You will need to
be connected to the Internet when you do this.

`sbt` runs in a Terminal. 

* To start a Terminal in VSCode, select `Terminal -> New Terminal`

You can also use a Terminal window outside of VSCode.  Any shell will work:
`bash`, `CMD.exe`, Powershell, etc.

From a Terminal, launch `sbt` using the command you set up during `sbt`
installation.  This is usually just `sbt`.  For example:

    terminal$ sbt
    [info] welcome to sbt ... (Amazon.com Inc. Java ...)
    [info] loading settings for project hw from build.sbt ...
    [info] set current project to Concepts of Programming Languages ...
    [info] started sbt server
    sbt> 

NOTE: you **must** be in the directory that contains the file `build.sbt` and
has the directory `src` when you run the `sbt` command.  Use `cd` to change to
that directory if necessary.  The `build.sbt` file contains instructions for
building and testing the project.  It will compile code into a new `target`
subdirectory.

You need to have Internet connectivity when you initially ask `sbt` to compile
and test in order to download to libraries.

## Launch Scala Console from `sbt`

To launch Scala from `sbt`, run:

    sbt> console
    scala>

To exit Scala and return to `sbt`, enter:

    scala> :quit
    sbt>

You will have access to code in `src/main/scala`.  For example, you can run:

    scala> import fp1.*
    scala> fact (5)

You can run files in the `play` directory by loading them:

    scala> :load play/README.scala
    

## Compiling with `sbt`

To compile the Scala source code with `sbt`, run:

    sbt> compile
    
To continuously compile, run:

    sbt> ~compile
    
For the assignments, you may prefer to simply run the tests as you go. However,
for tricky exercises, you may wish to edit source code, compile it from `sbt`,
and try it out in the Scala console.

## Testing with `sbt`

From `sbt`, you can run various commands to compile and run the tests under
`src/test`.

When you ask `sbt` to run any tests, it will automatically compile your source
code and the test code first (if it does not appear to do so, ensure that you
have saved your source code file in your text editor!).

To run all tests from `sbt`:

    sbt> test
    
To run all tests for the first functional programming assignment from `sbt`:
    
    sbt> testOnly fp1tests

The names of the tests for the other Scala assignments can be found by looking
under `src/test`.

To run the tests for exercise 5 from the first functional programming
assignment, run the following command from `sbt`:

    sbt> testOnly fp1tests -- -n fp1ex05

The most useful mode is to prefix the test command of your choice with ~ (the
tilde symbol).  That will cause `sbt` to continually compile and test your code
whenever it detects changes have been made. That is, for the first functional
programming assignment use:

    sbt> ~testOnly fp1tests

# Homework Instructions

For all assignments, you must submit source code that compiles without error.
Submissions where the source code does not compile will receive `0` points. If
source code compiles correctly, partial credit may be given for assignments
where some of the tests fail.  For Scala assignments, you can confirm that your
source code compiles correctly by running the following command in your
repository directory:

    sbt> compile

For each assignment, you will hand in a single source code file on D2L. Do not
hand in a zip file.

