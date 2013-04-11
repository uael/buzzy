Reproducible directory names.

  $ export HOME=/home/test
  $ export XDG_RUNTIME_DIR=/run/users/test
  $ unset XDG_CACHE_HOME
  $ unset XDG_CACHE_DIRS
  $ unset XDG_DATA_HOME
  $ unset XDG_DATA_DIRS

Print out the documentation for a handful of global variables.

  $ buzzy doc not_a_real_variable
  No variable named not_a_real_variable
  [1]

  $ buzzy doc cache_path
  cache_path
    A directory for user-specific nonessential data files
  
    On POSIX systems, this defaults to the value of the $XDG_CACHE_HOME environment variable, or $HOME/.cache if that's not defined.  Note that this is not a Buzzy-specific directory; this should refer to the root of the current user's cache directory.
  
    Current value: /home/test/.cache

  $ buzzy doc work_path
  work_path
    A directory for Buzzy's intermediate build products
  
    Current value: /home/test/.cache/buzzy


Then do the same for some repo- and package-specific variables, both while in a
repository and directory and while not in one.

  $ mkdir -p not-a-repo
  $ cd not-a-repo
  $ buzzy doc repo_base_path
  repo_base_path
    The base path of the files defining the repository
  $ buzzy doc repo_path
  repo_path
    The location of the YAML file defining the repository
  $ cd ..

  $ mkdir -p repo1/.buzzy
  $ cd repo1
  $ buzzy doc repo_base_path
  repo_base_path
    The base path of the files defining the repository
  
    Current value: .*/doc.t/repo1/.buzzy (re)
  $ buzzy doc repo_path
  repo_path
    The location of the YAML file defining the repository
  
    Current value: .*/doc.t/repo1/.buzzy/repo.yaml (re)
  $ cd ..

  $ mkdir -p repo2/.buzzy
  $ cat > repo2/.buzzy/package.yaml <<EOF
  > name: test
  > version: 1.0~rc1
  > EOF
  $ cd repo2
  $ buzzy doc repo_base_path
  repo_base_path
    The base path of the files defining the repository
  
    Current value: .*/doc.t/repo2/.buzzy (re)
  $ buzzy doc repo_path
  repo_path
    The location of the YAML file defining the repository
  
    Current value: .*/doc.t/repo2/.buzzy/repo.yaml (re)
  $ buzzy doc name
  name
    The name of the package
  
    Current value: test
  $ buzzy doc version
  version
    The version of the package
  
    Current value: 1.0~rc1
  $ buzzy doc source_path
  source_path
    Where the package's extracted source archive should be placed
  
    Current value: .*/doc.t/repo2/.buzzy/.. (re)
  $ cd ..