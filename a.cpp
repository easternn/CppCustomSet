#include <initializer_list>
#include <vector>
#include <algorithm>

template <class ValueType> class Set {
public:
    class iterator {
    public:
        const ValueType* item_ = nullptr;
        iterator* left_ = nullptr;
        iterator* right_ = nullptr;

        iterator() = default;

        explicit iterator(const ValueType* item_)
                : item_(item_) {
        }

        ~iterator() = default;

        Set::iterator& operator++() {
            *this = *right_;
            return *this;
        }

        Set::iterator operator++(int) {
            auto old = *this;
            *this = *right_;
            return old;
        }

        Set::iterator& operator--() {
            *this = *left_;
            return *this;
        }

        Set::iterator operator--(int) {
            auto old = *this;
            *this = *left_;
            return old;
        }

        const ValueType& operator*() {
            return *item_;
        }

        const ValueType* operator->() {
            return item_;
        }

        bool operator!=(const iterator& other) {
            return (item_ != other.item_);
        }

        bool operator==(const iterator& other) {
            return (item_ == other.item_);
        }
    };

    Set() {
        end_ = new iterator();
        begin_ = end_;
    }

    template<typename Iterator>
    Set(Iterator first, Iterator last) {
        end_ = new iterator();
        begin_ = end_;
        for (auto it = first; it != last; ++it) {
            insert(*it);
        }
    }

    Set(std::initializer_list<ValueType> elems) {
        end_ = new iterator();
        begin_ = end_;
        for (const auto& elem : elems) {
            insert(elem);
        }
    }

    Set(const Set<ValueType>& other) {
        new_tree(other);
    }

    Set<ValueType>& operator=(const Set<ValueType>& other) {
        if (&other == this) {
            return *this;
        }
        delete_all();
        new_tree(other);
        return *this;
    }

    ~Set() {
        delete_all();
    }

    std::size_t size() const {
        return size_;
    }

    bool empty() const {
        return (size_ == 0);
    }

    void insert(const ValueType& item) {
        auto* node = find_in_tree(item);
        if (node == nil_) {
            auto* item_copy = new ValueType(item);
            auto* it = new iterator(item_copy);
            insert_in_tree(item_copy, it);
            ++size_;
        }
        if (!black(root_)) {
            exit(1);
        }
    }

    void erase(const ValueType& item) {
        auto* node = find_in_tree(item);
        if (node != nil_) {
            erase_from_tree(node);
            --size_;
        }
        if (!black(root_)) {
            exit(1);
        }
    }

    iterator begin() const {
        return *begin_;
    }

    iterator end() const {
        return *end_;
    }

    iterator find(const ValueType& item) const {
        auto* node = find_in_tree(item);
        return (node == nil_) ? *end_ : *(node->it);
    }

    iterator lower_bound(const ValueType& item) const {
        auto* node = lower_bound_in_tree(item);
        return (node == nil_) ? *end_ : *(node->it);
    }

private:
    struct TreeNode {
        ValueType* item;
        iterator* it;
        TreeNode* left;
        TreeNode* right;
        TreeNode* parent;
        bool black;
        TreeNode(ValueType* item, iterator* it, TreeNode* left, TreeNode* right, TreeNode* parent, bool black)
                : item(item), it(it), left(left), right(right), parent(parent), black(black) {
        }
    };

    bool comp(const ValueType& a, const ValueType& b) const {
        return !(a < b) && !(b < a);
    }

    void new_tree(const Set<ValueType>& other) {
        end_ = new iterator();
        begin_ = end_;
        root_ = copy_tree(other.root_);
        size_ = other.size_;
        std::vector<TreeNode*> nodes = {};
        lr_dfs(root_, nodes);
        iterator* last = nullptr;
        for (auto* node : nodes) {
            if (last == nullptr) {
                begin_ = node->it;
            } else {
                node->it->left_ = last;
                last->right_ = node->it;
            }
            last = node->it;
        }
        if (!nodes.empty()) {
            auto* last_it = nodes.back()->it;
            last_it->right_ = end_;
            end_->left_ = last_it;
        }
    }

    TreeNode* copy_tree(TreeNode* other_node) {
        if (other_node == nil_) {
            return nil_;
        }
        const auto& item = *(other_node->item);
        auto* item_copy = new ValueType(item);
        auto* it = new iterator(item_copy);
        auto* node = new TreeNode(item_copy, it, nil_, nil_, nil_, other_node->black);
        node->left = copy_tree(other_node->left);
        if (node->left != nil_) {
            node->left->parent = node;
        }
        node->right = copy_tree(other_node->right);
        if (node->right != nil_) {
            node->right->parent = node;
        }
        return node;
    }

    void lr_dfs(TreeNode* node, std::vector<TreeNode*>& nodes) const {
        if (node == nil_) {
            return;
        }
        lr_dfs(node->left, nodes);
        nodes.push_back(node);
        lr_dfs(node->right, nodes);
    }

    void delete_node(TreeNode* node) {
        if (node == nil_) {
            return;
        }
        delete node->item;
        delete node->it;
        delete node;
    }

    void delete_tree(TreeNode* node) {
        if (node == nullptr) {
            return;
        }
        delete_tree(node->left);
        delete_tree(node->right);
        delete_node(node);
    }

    void delete_all() {
        delete_tree(root_);
        delete end_;
    }

    void insert_it(iterator* it) {
        if (it == nullptr) {
            return;
        }
        auto* next_node = lower_bound_in_tree(**it);
        auto* next_it = (next_node == nullptr) ? end_ : next_node->it;
        auto* prev_it = next_it->left_;
        it->left_ = prev_it;
        it->right_ = next_it;
        next_it->left_ = it;
        if (prev_it) {
            prev_it->right_ = it;
        } else {
            begin_ = it;
        }
    }

    void extract_it(iterator* it) {
        if (it == nullptr) {
            return;
        }
        auto* prev_it = it->left_;
        auto* next_it = it->right_;
        if (begin_ == it) {
            begin_ = next_it;
        }
        if (prev_it) {
            prev_it->right_ = next_it;
        }
        if (next_it) {
            next_it->left_ = prev_it;
        }
    }

    bool black(TreeNode* node) const {
        return (node == nil_) || node->black;
    }

    TreeNode* find_brother(TreeNode* node) const {
        auto* parent = node->parent;
        return (parent->left == node) ? parent->right : parent->left;
    }

    void rotate(TreeNode* node) {
        auto* parent = node->parent;
        if (node == parent->right) {
            parent->right = node->left;
            if (node->left != nil_) {
                node->left->parent = parent;
            }
            node->left = parent;
        } else {
            parent->left = node->right;
            if (node->right != nil_) {
                node->right->parent = parent;
            }
            node->right = parent;
        }
        node->parent = parent->parent;
        if (node->parent != nullptr) {
            if (parent == node->parent->left) {
                node->parent->left = node;
            } else {
                node->parent->right = node;
            }
        }
        parent->parent = node;
        if (node->parent == nullptr) {
            root_ = node;
        }
    }

    TreeNode* find_in_tree(const ValueType& item) const {
        auto* node = root_;
        while (node != nil_) {
            if (comp(item, *(node->item))) {
                return node;
            } else if (item < *(node->item)) {
                node = node->left;
            } else {
                node = node->right;
            }
        }
        return nil_;
    }

    TreeNode* lower_bound_in_tree(const ValueType& item) const {
        auto* node = root_;
        auto* ans = nil_;
        while (node != nil_) {
            if ((item < *(node->item)) || comp(*(node->item), item)) {
                ans = node;
                node = node->left;
            } else {
                node = node->right;
            }
        }
        return ans;
    }

    void fix_tree_after_inserting(TreeNode* node) {
        while (!black(node->parent)) {
            auto* parent = node->parent;
            auto* grand = parent->parent;
            auto* uncle = (grand->left == parent) ? grand->right : grand->left;
            if ((parent == grand->left && node == parent->right) || (parent == grand->right && node == parent->left)) {
                rotate(node);
                std::swap(parent, node);
            }
            if (black(uncle)) {
                rotate(parent);
                grand->black = false;
                parent->black = true;
                break;
            } else {
                grand->black = false;
                parent->black = true;
                uncle->black = true;
                node = grand;
            }
        }
        root_->black = true;
    }

    void insert_in_tree(ValueType* item, iterator* it) {
        auto* node = new TreeNode(item, it, nil_, nil_, nil_, false);
        insert_it(it);
        if (this->empty()) {
            node->black = true;
            root_ = node;
            return;
        }
        auto* parent = root_;
        while (node->parent == nil_) {
            if (*(parent->item) < *(node->item)) {
                if (parent->right == nil_) {
                    node->parent = parent;
                    parent->right = node;
                } else {
                    parent = parent->right;
                }
            } else {
                if (parent->left == nil_) {
                    node->parent = parent;
                    parent->left = node;
                } else {
                    parent = parent->left;
                }
            }
        }
        fix_tree_after_inserting(node);
    }

    void fix_tree_for_erasing(TreeNode* node) {
        while (node != root_ && black(node)) {
            auto* parent = node->parent;
            auto* brother = find_brother(node);
            if (!black(brother)) {
                parent->black = false;
                brother->black = true;
                rotate(brother);
                brother = find_brother(node);
            }
            auto* left_nephew = (brother == nil_) ? nil_ : brother->left;
            auto* right_nephew = (brother == nil_) ? nil_ : brother->right;
            if (brother == nil_ || (black(left_nephew) && black(right_nephew))) {
                bool black_parent = parent->black;
                parent->black = true;
                if (brother != nil_) {
                    brother->black = false;
                }
                node = parent;
                if (!black_parent) {
                    break;
                }
                continue;
            }
            if (node == parent->left) {
                if (black(right_nephew)) {
                    left_nephew->black = true;
                    brother->black = false;
                    rotate(left_nephew);

                    brother = find_brother(node);
                    left_nephew = brother->left;
                    right_nephew = brother->right;
                }
                brother->black = parent->black;
                parent->black = true;
                right_nephew->black = true;
                rotate(brother);
            } else {
                if (black(left_nephew)) {
                    right_nephew->black = true;
                    brother->black = false;
                    rotate(right_nephew);

                    brother = find_brother(node);
                    left_nephew = brother->left;
                    right_nephew = brother->right;
                }
                brother->black = parent->black;
                parent->black = true;
                left_nephew->black = true;
                rotate(brother);
            }
            break;
        }
    }

    void erase_from_tree(TreeNode* node) {
        if (size_ == 1) {
            extract_it(node->it);
            delete_node(node);
            root_ = nullptr;
            return;
        }
        if (node->left != nil_ && node->right != nil_) {
            auto* old_node = node;
            node = node->right;
            while (node->left != nil_) {
                node = node->left;
            }
            std::swap(old_node->item, node->item);
            std::swap(old_node->it, node->it);
        }
        fix_tree_for_erasing(node);
        auto* new_child = (node->left == nil_) ? node->right : node->left;
        auto* new_parent = node->parent;
        if (new_child != nil_) {
            new_child->parent = new_parent;
        }
        extract_it(node->it);
        delete_node(node);
        if (new_parent == nil_) {
            root_ = new_child;
        } else if (node == new_parent->left) {
            new_parent->left = new_child;
        } else {
            new_parent->right = new_child;
        }
        root_->black = true;
    }

    iterator* begin_ = nullptr;
    iterator* end_ = nullptr;
    TreeNode* root_ = nullptr;
    TreeNode* nil_ = nullptr;
    std::size_t size_ = 0;
};
