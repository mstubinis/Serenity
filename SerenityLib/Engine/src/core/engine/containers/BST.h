#pragma once
#ifndef ENGINE_CONTAINERS_BST_H
#define ENGINE_CONTAINERS_BST_H

namespace Engine {
    template<typename T> class BST {
        struct Node final {
            T data;
            Node* left   = nullptr;
            Node* right  = nullptr;
            Node(T inData) 
                : data{ inData }
            {}
        };
        private:
            Node* m_Root = nullptr;

            void internal_cleanup() {
                if (!m_Root) {
                    return;
                }
                //bfs cleanup
                std::queue<Node*> q;
                q.push(m_Root);
                while (!q.empty()) {
                    auto* curr = q.front();
                    q.pop();

                    if (curr->left) {
                        q.push(curr->left);
                    }
                    if (curr->right) {
                        q.push(curr->right);
                    }
                    delete curr;
                }
                m_Root = nullptr;
            }
       
            Node* internal_recurse_removal(Node* root, T inData) noexcept {
                if (!root) {
                    return root;
                }
                if (root->data == inData) {
                    if (!root->left && !root->right) {
                        delete root;
                        return nullptr;
                    }else if ((root->left && !root->right) || (!root->left && root->right)) {
                        auto* temp = (root->left) ? root->left : root->right;
                        delete root;
                        return temp;
                    }else {
                        auto* temp = root->left;
                        while (temp->right)
                            temp = temp->right;
                        T nextAvail = temp->data;
                        root->data = nextAvail;
                        root->left = internal_recurse_removal(root->left, nextAvail);
                    }
                }else if (inData < root->data) {
                    root->left = internal_recurse_removal(root->left, inData);
                }else{
                    root->right = internal_recurse_removal(root->right, inData);
                }
                return root;
            }
        public:
            BST() = default;
            virtual ~BST() {
                internal_cleanup();
            }
            void remove(T data) noexcept {
                internal_recurse_removal(m_Root, data);
            }
            void insert(T data) noexcept {
                if (!m_Root) {
                    m_Root = new Node(data);
                    return;
                }
                Node* curr = m_Root;
                while (curr) {
                    if (data <= curr->data) {
                        if(curr->left){
                            curr = curr->left;
                        }else{
                            curr->left = new Node(data);
                            return;
                        }
                    }else{
                        if (curr->right) {
                            curr = curr->right;
                        }else{
                            curr->right = new Node(data);
                            return;
                        }
                    }
                }
            }
            void print_inorder() noexcept {
                std::cout << "[";
                std::function<void(Node*)> recurse = [&](Node* root) -> void {
                    if (!root) {
                        return;
                    }
                    recurse(root->left);
                    std::cout << ", " << root->data;
                    recurse(root->right);
                };
                recurse(m_Root);
                std::cout << "]\n";
            }
            void print_preorder() noexcept {
                std::cout << "[";
                std::function<void(Node*)> recurse = [&](Node* root) -> void {
                    if (!root) {
                        return;
                    }
                    std::cout << ", " << root->data;
                    recurse(root->left);
                    recurse(root->right);
                };
                recurse(m_Root);
                std::cout << "]\n";
            }
            void print_postorder() noexcept {
                std::cout << "[";
                std::function<void(Node*)> recurse = [&](Node* root) -> void {
                    if (!root) {
                        return;
                    }
                    recurse(root->left);
                    recurse(root->right);
                    std::cout << ", " << root->data;
                };
                recurse(m_Root);
                std::cout << "]\n";
            }
    };
};

#endif