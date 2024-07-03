# gitignore

这个gitignore文件很容易导致错误，比如会把他自己也忽略掉

```gitignore
# Ignore everything
*

# Do not ignore directories, so we can traverse into them
!*/

# Ignore everything in the root directory
/*

# Do not ignore the Core directory and its contents
!Core/
!Core/**

# Do not ignore .ioc files
!*.ioc
!*.md

!docs/
!docs/**

```
