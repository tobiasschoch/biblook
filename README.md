# Biblook/ bibindex

### Summary

**Biblook** is a program for quickly searching through bibtex files. The simplest thing `biblook` allows you do is find all entries with a certain word in a certain field ("`find author twain`"). Search results can be displayed or saved into another bibtex file.

**Bibindex** is a support program that creates an index file for later use by `biblook`. You run `bibindex` once for each bibtex file you have. The indexing makes searching with `biblook` much faster than using something like grep or emacs.

### Credits and License

`biblook` and `bibindex` were written by Jeff Erickson with numerous additions and bug fixes by Nelson Beebe, Sariel Har-Peled, Bill Jones, Erik Schoenfelder, and Rafael Laboissiere. The programs are in the public domain. Rafael Laboissiere licensed the code under GNU General Public License (April 5, 2000; see [Link](https://www.cs.usask.ca/ftp/pub/geometry/geombib/bibtools/)).

In version 2.11, I fixed some compiler warnings, added color to the output on the console (see flag `WITH_COLOR`), and added a `Makefile` for `gcc` and a POSIX-compliant environment. The original `Makefile` has been renamed to `Makefile_original`.

### Community guidelines

###### Submitting an issue

If you have any suggestions for feature additions or any problems with the software that you would like addressed with the development community, please submit an issue on the Issues tab of the project GitHub repository. You may want to search the existing issues before submitting, to avoid asking a question or requesting a feature that has already been discussed.

###### How to contribute

If you are interested in modifying the code, you may fork the project for your own use, as detailed in the GNU General Public License we have adopted for the project. In order to contribute, please contact the maintainer by Tobias Schoch at gmail dot com (the names are separated by a dot) after making the desired changes.
