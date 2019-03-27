#include <core/engine/mesh/MeshRequest.h>

using namespace std;

MeshRequest::MeshRequest() {
    file = "";
}
MeshRequest::MeshRequest(std::string filename) {
    file = filename;
}
MeshRequest::~MeshRequest() {

}
void MeshRequest::request() {
    if (file != "") {
        
    }
}