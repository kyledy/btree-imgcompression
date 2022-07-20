/*
*  File:        ptree.cpp
*  Description: Implementation of a partitioning tree class for CPSC 221 PA3
*  Date:        2022-03-03 01:53
*
*               ADD YOUR PRIVATE FUNCTION IMPLEMENTATIONS TO THE BOTTOM OF THIS FILE
*/

#include "ptree.h"
#include "hue_utils.h" // useful functions for calculating hue averages
#include <iostream>
#include <cassert>

using namespace cs221util;
using namespace std;

// The following definition may be convenient, but is not necessary to use
typedef pair<unsigned int, unsigned int> pairUI;

/////////////////////////////////
// PTree private member functions
/////////////////////////////////

/*
*  Destroys all dynamically allocated memory associated with the current PTree object.
*  You may want to add a recursive helper function for this!
*  POST: all nodes allocated into the heap have been released.
*/
void PTree::Clear() {
  // add your implementation below
  Clear(root);
  root = NULL;
}

/*
*  Copies the parameter other PTree into the current PTree.
*  Does not free any memory. Should be called by copy constructor and operator=.
*  You may want a recursive helper function for this!
*  PARAM: other - the PTree which will be copied
*  PRE:   There is no dynamic memory associated with this PTree.
*  POST:  This PTree is a physically separate copy of the other PTree.
*/
void PTree::Copy(const PTree& other) {
  // add your implementation below
  Copy(other.root);
}

/*
*  Private helper function for the constructor. Recursively builds the tree
*  according to the specification of the constructor.
*  You *may* change this if you like, but we have provided here what we
*  believe will be sufficient to use as-is.
*  PARAM:  im - full reference image used for construction
*  PARAM:  ul - upper-left image coordinate of the currently building Node's image region
*  PARAM:  w - width of the currently building Node's image region
*  PARAM:  h - height of the currently building Node's image region
*  RETURN: pointer to the fully constructed Node
*/
Node* PTree::BuildNode(PNG& im, pair<unsigned int, unsigned int> ul, unsigned int w, unsigned int h) {

  if (root == nullptr) // base case 1
    return nullptr;

  if (w == 1 && h == 1) { // base case 2
    Node* leafNode = new Node(ul, w, h, GetAverageOfRegion(im, ul.first, ul.second, w, h));
    leafNode->A = nullptr;
    leafNode->B = nullptr;
    return leafNode;
  }

  Node* newNode = new Node(ul, w, h, GetAverageOfRegion(im, ul.first, ul.second, w, h));
    
  if (h > w) { // if current node is taller than it is wide

    if (h % 2 == 0) { // if the image can be divided symmetrically
      newNode->A = BuildNode(im, ul, w, h / 2);
      newNode->B = BuildNode(im, make_pair(ul.first, ul.second + (h / 2)), w, (h / 2));
    } else {
      newNode->A = BuildNode(im, ul, w, (h / 2));
      newNode->B = BuildNode(im, make_pair(ul.first, ul.second + (h / 2)), w, 
      (h / 2) + (h % 2));
    }


  } else { // image is wider than it is tall, or square
    
    if (w % 2 == 0) { // if the image can be divided symmetrically
      newNode->A = BuildNode(im, ul, w / 2, h);
      newNode->B = BuildNode(im, make_pair(ul.first + (w / 2), ul.second), (w / 2), h);
    } else {
      newNode->A = BuildNode(im, ul, (w / 2), h);
      newNode->B = BuildNode(im, make_pair(ul.first + (w / 2), ul.second), (w / 2) + (w % 2), 
      h);
    }
  }

  return newNode;
}

////////////////////////////////
// PTree public member functions
////////////////////////////////

/*
*  Constructor that builds the PTree using the provided PNG.
*
*  The PTree represents the sub-image (actually the entire image) from (0,0) to (w-1, h-1) where
*  w-1 and h-1 are the largest valid image coordinates of the original PNG.
*  Each node corresponds to a rectangle of pixels in the original PNG, represented by
*  an (x,y) pair for the upper-left corner of the rectangle, and two unsigned integers for the
*  number of pixels on the width and height dimensions of the rectangular sub-image region the
*  node defines.
*
*  A node's two children correspond to a partition of the node's rectangular region into two
*  equal (or approximately equal) size regions which are either tiled horizontally or vertically.
*
*  If the rectangular region of a node is taller than it is wide, then its two children will divide
*  the region into vertically-tiled sub-regions of equal height:
*  +-------+
*  |   A   |
*  |       |
*  +-------+
*  |   B   |
*  |       |
*  +-------+
*
*  If the rectangular region of a node is wider than it is tall, OR if the region is exactly square,
*  then its two children will divide the region into horizontally-tiled sub-regions of equal width:
*  +-------+-------+
*  |   A   |   B   |
*  |       |       |
*  +-------+-------+
*
*  If any region cannot be divided exactly evenly (e.g. a horizontal division of odd width), then
*  child B will receive the larger half of the two subregions.
*
*  When the tree is fully constructed, each leaf corresponds to a single pixel in the PNG image.
* 
*  For the average colour, this MUST be computed separately over the node's rectangular region.
*  Do NOT simply compute this as a weighted average of the children's averages.
*  The functions defined in hue_utils.h and implemented in hue_utils.cpp will be very useful.
*  Computing the average over many overlapping rectangular regions sounds like it will be
*  inefficient, but as an exercise in theory, think about the asymptotic upper bound on the
*  number of times any given pixel is included in an average calculation.
* 
*  PARAM: im - reference image which will provide pixel data for the constructed tree's leaves
*  POST:  The newly constructed tree contains the PNG's pixel data in each leaf node.
*/
PTree::PTree(PNG& im) {

  root = new Node(make_pair(0,0), im.width(), im.height(), GetAverageOfRegion(im, 0, 0, im.width(), im.height()));

  /* CASES FOR CONSTRUCTOR 

  1) image is taller than it is wide (height > width)
    - divide the image on a horizontal axis, which bisects the height into two halves 

  2) image is wider than it is tall, or square 
    - divide the image on a vertical axis, which bisects the width into two halves 

  */

  if (im.height() > im.width()) { // image is taller than it is wide

     int heightOfRegion = im.height() / 2;
     int remainderH = im.height() % 2;

     if (remainderH == 0) { // if the image can be divided symmetrically
       root->A = BuildNode(im, root->upperleft, im.width(), heightOfRegion);
       root->B = BuildNode(im, make_pair(root->upperleft.first, root->upperleft.second + heightOfRegion), im.width(), heightOfRegion);
     } else {
       root->A = BuildNode(im, root->upperleft, im.width(), heightOfRegion);
       root->B = BuildNode(im, make_pair(root->upperleft.first, root->upperleft.second + heightOfRegion), im.width(), 
       heightOfRegion + remainderH);
     }

   } else { // image is wider than it is tall, or square
    
    int widthOfRegion = im.width() / 2;   
    int remainderW = im.width() % 2;

    if (remainderW == 0) { // if the image can be divided symmetrically
      root->A = BuildNode(im, root->upperleft, widthOfRegion, im.height());
      root->B = BuildNode(im, make_pair(root->upperleft.first + widthOfRegion, root->upperleft.second), widthOfRegion, 
      im.height());
    } else {
      root->A = BuildNode(im, root->upperleft, widthOfRegion, im.height());
      root->B = BuildNode(im, make_pair(root->upperleft.first + widthOfRegion, root->upperleft.second), 
      widthOfRegion + remainderW, im.height());
    }
  }
}

/*
*  Copy constructor
*  Builds a new tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  This tree is constructed as a physically separate copy of other tree.
*/
PTree::PTree(const PTree& other) {
  // add your implementation below
  root = Copy(other.root);
}

/*
*  Assignment operator
*  Rebuilds this tree as a copy of another tree.
*
*  PARAM: other - an existing PTree to be copied
*  POST:  If other is a physically different tree in memory, all pre-existing dynamic
*           memory in this tree is deallocated and this tree is reconstructed as a
*           physically separate copy of other tree.
*         Otherwise, there is no change to this tree.
*/
PTree& PTree::operator=(const PTree& other) {
  // add your implementation below
  if (this != &other) {
    Clear(root);
    root = Copy(other.root);
  } 
  return *this;
}

/*
*  Destructor
*  Deallocates all dynamic memory associated with the tree and destroys this PTree object.
*/
PTree::~PTree() {
  // add your implementation below
  Clear();
}



/*
*  Traverses the tree and puts the leaf nodes' color data into the nodes'
*  defined image regions on the output PNG.
*  For non-pruned trees, each leaf node corresponds to a single pixel that will be coloured.
*  For pruned trees, each leaf node may cover a larger rectangular region that will be
*  entirely coloured using the node's average colour attribute.
*
*  You may want to add a recursive helper function for this!
*
*  RETURN: A PNG image of appropriate dimensions and coloured using the tree's leaf node colour data
*/
PNG PTree::Render() const {
  // replace the line below with your implementation
  PNG* outpng = new PNG(root->width, root->height);
  Render(root, outpng); // Recursive helper function is void, only modifies the image data
  return *outpng;
}

/*
*  Trims subtrees as high as possible in the tree. A subtree is pruned
*  (its children are cleared/deallocated) if ALL of its leaves have colour
*  within tolerance of the subtree root's average colour.
*  Pruning criteria should be evaluated on the original tree, and never on a pruned
*  tree (i.e. we expect that Prune would be called on any tree at most once).
*  When processing a subtree, you should determine if the subtree should be pruned,
*  and prune it if possible before determining if it has subtrees that can be pruned.
* 
*  You may want to add (a) recursive helper function(s) for this!
*
*  PRE:  This tree has not been previously pruned (and is not copied/assigned from a tree that has been pruned)
*  POST: Any subtrees (as close to the root as possible) whose leaves all have colour
*        within tolerance from the subtree's root colour will have their children deallocated;
*        Each pruned subtree's root becomes a leaf node.
*/
void PTree::Prune(double tolerance) {
  // add your implementation below
  Prune(root, tolerance);
}

/*
*  Returns the total number of nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::Size() const {
  // replace the line below with your implementatioN
  return Size(root);
}

/*
*  Returns the total number of leaf nodes in the tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*/
int PTree::NumLeaves() const {
  // replace the line below with your implementation
  return NumLeaves(root);
}

/*
*  Rearranges the nodes in the tree, such that a rendered PNG will be flipped horizontally
*  (i.e. mirrored over a vertical axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped horizontally.
*/
void PTree::FlipHorizontal() {
  // add your implementation below
  FlipHorizontal(root);
}

/*
*  Like the function above, rearranges the nodes in the tree, such that a rendered PNG
*  will be flipped vertically (i.e. mirrored over a horizontal axis).
*  This can be achieved by manipulation of the nodes' member attribute(s).
*  Note that this may possibly be executed on a pruned tree.
*  This function should run in time linearly proportional to the size of the tree.
*
*  You may want to add a recursive helper function for this!
*
*  POST: Tree has been modified so that a rendered PNG will be flipped vertically.
*/
void PTree::FlipVertical() {
  // add your implementation below
  FlipVertical(root);
}

/*
    *  Provides access to the root of the tree.
    *  Dangerous in practice! This is only used for testing.
    */
Node* PTree::GetRoot() {
  return root;
}

//////////////////////////////////////////////
// PERSONALLY DEFINED PRIVATE MEMBER FUNCTIONS
//////////////////////////////////////////////

// Helper function for BuildNode
HSLAPixel PTree::GetAverageOfRegion(PNG& im, unsigned int xCoord, unsigned int yCoord, unsigned int w, unsigned int h) {
  
  double dimensions = w * h; 

  double avgXHue = 0.0;
  double avgYHue = 0.0;

  double avgSat = 0.0;
  double avgLum = 0.0;
  double avgAlp = 0.0;
  
  for (unsigned int x = 0; x < w; x++) {
    for (unsigned int y = 0; y < h; y++) {
      HSLAPixel* pixel = im.getPixel(x + xCoord, y + yCoord);
      
      double xHue = Deg2X(pixel->h);
      double yHue = Deg2Y(pixel->h);
      avgXHue += xHue;
      avgYHue += yHue;
      
      avgSat += pixel->s;
      avgLum += pixel->l;
      avgAlp += pixel->a;
    }
  }

  avgXHue = avgXHue / dimensions;
  avgYHue = avgYHue / dimensions;  

  double avgHue = XY2Deg(avgXHue, avgYHue);
  avgSat = (double) avgSat / (double) dimensions;
  avgLum = (double) avgLum / (double) dimensions;
  avgAlp = (double) avgAlp / (double) dimensions;

  return HSLAPixel(avgHue, avgSat, avgLum, avgAlp);
}

// Recursive helper function for copy constructor
Node* PTree::Copy(const Node* subRoot) 
{
  if (subRoot == nullptr)
    return nullptr;

  Node *newNode = new Node(subRoot->upperleft, subRoot->width, subRoot->height, subRoot->avg);
  newNode->A = Copy(subRoot->A);
  newNode->B = Copy(subRoot->B);
  return newNode;  
}

// Recursive helper function for Clear()
void PTree::Clear(const Node* subRoot) const
{
  if (subRoot == NULL)
    return;

  Clear(subRoot->A);
  Clear(subRoot->B);
  delete subRoot;   
}

// Recursive helper function for Render()
void PTree::Render(const Node* subRoot, PNG* img) const
{
  if (subRoot == nullptr)
    return;

  if (subRoot->A == nullptr && subRoot->B == nullptr) { // if this node is a leaf
    
    if (subRoot->width == 1 && subRoot->height == 1) { // if the tree is non-pruned, all leaves have width & height 1
      HSLAPixel *pixel = img->getPixel(subRoot->upperleft.first, subRoot->upperleft.second);
      *pixel = subRoot->avg;
    } else { // otherwise if the tree is pruned
      
      // x and y coordinates of the region represented by the leaf node
      unsigned int x = subRoot->upperleft.first; 
      unsigned int y = subRoot->upperleft.second; 

      for (unsigned int i = x; i < x + subRoot->width; i++) {
        for (unsigned int j = y; j < y + subRoot->height; j++) {
          HSLAPixel *pixel = img->getPixel(i, j);
          *pixel = subRoot->avg;
        }
      }

    }
  }

  Render(subRoot->A, img);
  Render(subRoot->B, img);
}

// Recursive helper function for Prune()
void PTree::Prune(Node* subRoot, double tolerance) {
  
  if (subRoot == nullptr) 
    return;

  HSLAPixel avg = subRoot->avg;  

  // If the current subtree should be pruned, then deallocate its children, making this node a leaf node
  if (shouldBePruned(subRoot->A, avg, tolerance) && shouldBePruned(subRoot->B, avg, tolerance)) {
    Clear(subRoot->A);
    Clear(subRoot->B);
    
    // Not sure if the code below is necessary, but just putting it to avoid a memory leak? Not sure if not putting this before
    // the call to Clear() will result in a memory leak though
    subRoot->A = nullptr;
    subRoot->B = nullptr;
  }

  Prune(subRoot->A, tolerance);
  Prune(subRoot->B, tolerance);
}

int PTree::shouldBePruned(Node* subRoot, HSLAPixel avg, double tolerance) {

  if (subRoot == nullptr) 
    return 1;
  
  if (subRoot->A == nullptr && subRoot->B == nullptr) { // if this node is a leaf
    return isWithinTolerance(avg, subRoot->avg, tolerance);
  }

  return shouldBePruned(subRoot->A, avg, tolerance) && shouldBePruned(subRoot->B, avg, tolerance);
}

// Helper function for shouldBePruned()
int PTree::isWithinTolerance(HSLAPixel avg, HSLAPixel toCompare, double tolerance) {
  return avg.dist(toCompare) <= tolerance;
}

// Recursive helper function for Size()
int PTree::Size(const Node* subRoot) const
{
  if (subRoot == NULL) 
    return 0;

  return 1 + Size(subRoot->A) + Size(subRoot->B);  
}

// Recursive helper function for NumLeaves()
int PTree::NumLeaves(const Node* subRoot) const 
{
  if (subRoot == nullptr) 
    return 0;

  if (subRoot->A == nullptr && subRoot->B == nullptr) {
    return 1;
  } else {
    return NumLeaves(subRoot->A) + NumLeaves(subRoot->B);
  }
}

// Recursive helper function FlipHorizontal()
void PTree::FlipHorizontal(Node* subRoot)
{
 if (subRoot == nullptr) 
   return;

 if (subRoot->A == nullptr && subRoot->B == nullptr) { // if this node is a leaf 
   
   if (subRoot->width == 1 && subRoot->height == 1) { // this tree is non-pruned
    int newXCoord = root->width - subRoot->upperleft.first - 1;
    subRoot->upperleft = make_pair(newXCoord, subRoot->upperleft.second);
   } else { // if it is a pruned tree
    int newXCoord = root->width - subRoot->width - subRoot->upperleft.first; 
    subRoot->upperleft = make_pair(newXCoord, subRoot->upperleft.second);
   }
 }

 FlipHorizontal(subRoot->A);
 FlipHorizontal(subRoot->B);
}

// Recursive helper function FlipVertical()
void PTree::FlipVertical(Node* subRoot)
{
 if (subRoot == nullptr) 
   return;

 if (subRoot->A == nullptr && subRoot->B == nullptr) { // if this node is a leaf 
   
   if (subRoot->width == 1 && subRoot->height == 1) { // this tree is non-pruned
    int newYCoord = root->height - subRoot->upperleft.second - 1;
    subRoot->upperleft = make_pair(subRoot->upperleft.first, newYCoord);
   } else { // if it is a pruned tree
    int newYCoord = root->height - subRoot->height - subRoot->upperleft.second; 
    subRoot->upperleft = make_pair(subRoot->upperleft.first, newYCoord);
   }
 }

 FlipVertical(subRoot->A);
 FlipVertical(subRoot->B);
}