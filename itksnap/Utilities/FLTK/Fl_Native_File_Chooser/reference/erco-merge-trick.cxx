/*=========================================================================

   Copyright 2010, Dr. Sandra Black
   Linda C. Campbell Cognitive Neurology Unit
   Sunnybrook Health Sciences Center

   This file is part of ITK-SNAP -SB

   ITK-SNAP-SB is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

   ------
=========================================================================*/
#define FLTK1

#include <stdio.h>

#ifdef FLTK1
#define FOO_CLASS Fl_Foo
#else
#define FOO_CLASS fltk::Foo
#endif

#ifdef FLTK1
class Fl_Foo {
#else
namespace fltk {
class Foo {
#endif

public:
    void aaa();
    void bbb();

#ifdef FLTK1
};
#else
};
}
#endif

void FOO_CLASS::aaa() {
   printf("aaa\n");
}

void FOO_CLASS::bbb() {
   printf("bbb\n");
}

#ifdef FLTK1
int main() {
   Fl_Foo foo;
   foo.aaa();
   foo.bbb();
}
#else
int main() {
   fltk::Foo foo;
   foo.aaa();
   foo.bbb();
}
#endif
