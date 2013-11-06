Short: AVL Balanced Binary Tree with Duplicates handling and GUI Demo.
Author: agmsmith@achilles.net (Alexander G. M. Smith)
Uploader: agmsmith@achilles.net (Alexander G. M. Smith)
Website: http://www.achilles.net/~agmsmith/
Version: 1.0
Type: Development/Example Code
Requires: BeOS 5.0+

AVLDupTree is a set of C subroutines (not C++, so you can use it in drivers) that is useful for indexing a set of key/value pairs, using the key to find a matching value.  The standard AVL balanced binary tree algorithm is enhanced to support multiple values for the same key.  It is designed for future use in a file system to support fast attribute indexing and queries, but you can use it for other things.

The API supports these operations:

Create a tree, with a specified data type for keys and another for values (choose from C string (any length), int32, int64, float, double).  Optionally enable multitasking protection - which allows N simultaneous readers or one writer.

Deallocate a tree and its contents.

Add a key/value pair.

Delete a key/value pair.

Iterate over the tree.  This uses a callback function for efficiency, so you can process a large batch of key/values in one operation rather than having a "find" operation to find individual ones.  It is generalized to efficiently iterate over an optionally open ended range of keys, optionally including the ones equal to your range limits (the difference between less-than-or-equal and less-than), making it ideal for query processing.


AGMSAVLTest is a BeOS GUI program for testing the tree library and demonstrating the tree operations via a graphical display of the tree.  It also has a cool subtle colour cycling effect.


AVLDupTree is released under the GNU Lesser General Public License.  The AGMSAVLTest program is released as public domain.

- Alex (Ottawa, March 2001)
