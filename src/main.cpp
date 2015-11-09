#include <string>

#include "shape_tree.h"

int main() {

    ShapeTree shapeTree;

    shapeTree.initFromFile(std::string("plane.off"));
    Node* root = shapeTree.getRoot();

    std::vector<Node*> split1, split2;

    std::vector<double> weights(3,1.);

    root->split(X, split1, weights);

    split1[0]->extrude(5);
    split1[1]->extrude(3);
    split1[2]->extrude(2);

    split1[1]->split(Y, split2, weights);

    shapeTree.displayGeometry();
    
    return 0;
}

