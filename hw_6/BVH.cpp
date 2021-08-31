#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        

        std::vector<Object*> leftshapes;
        std::vector<Object*> rightshapes;

        if (splitMethod == SplitMethod::NAIVE) {
            auto beginning = objects.begin();
            auto middling = objects.begin() + (objects.size() / 2);
            auto ending = objects.end();

            leftshapes = std::vector<Object*>(beginning, middling);
            rightshapes = std::vector<Object*>(middling, ending);
        }
        else if (splitMethod == SplitMethod::SAH) {
            double min_in_dim;
            double max_in_dim;
            switch (dim) {
            case 0:
                min_in_dim = (objects[0]->getBounds().Centroid()).x;
                max_in_dim = (objects[objects.size() - 1]->getBounds().Centroid()).x;
                break;
            case 1:
                min_in_dim = (objects[0]->getBounds().Centroid()).y;
                max_in_dim = (objects[objects.size() - 1]->getBounds().Centroid()).y;
                break;
            case 2:
                min_in_dim = (objects[0]->getBounds().Centroid()).z;
                max_in_dim = (objects[objects.size() - 1]->getBounds().Centroid()).z;
                break;
            }
            Bounds3 bound;

            constexpr int bucket_num = 12;
            struct bucket {
                unsigned int count = 0;
                Bounds3 bound;
            };
            bucket buckets[bucket_num];

            unsigned int now_bucket_index = 0;
            for (int i = 0; i < objects.size(); i++) {
                bound = Union(bound, objects[i]->getBounds());

                double centroid;
                switch (dim) {
                case 0:
                    centroid = (objects[i]->getBounds().Centroid()).x;
                    break;
                case 1:
                    centroid = (objects[i]->getBounds().Centroid()).y;
                    break;
                case 2:
                    centroid = (objects[i]->getBounds().Centroid()).z;
                    break;
                }
                if (centroid > (min_in_dim + (now_bucket_index + 1) * (max_in_dim - min_in_dim) / bucket_num)) {
                    now_bucket_index++;
                }
                buckets[now_bucket_index].count++;
                buckets[now_bucket_index].bound = Union(buckets[now_bucket_index].bound, objects[i]->getBounds());
            }

            unsigned int min_left_count = 0;
            double min_cost = DBL_MAX;
            for (int split = 1; split < bucket_num - 1; split++) {
                Bounds3 left_bound, right_bound;
                unsigned int left_count = 0, right_count = 0;
                for (int left_index = 0; left_index < split; left_index++) {
                    left_count += buckets[left_index].count;
                    left_bound = Union(left_bound, buckets[left_index].bound);
                }
                for (int right_index = split; right_index < bucket_num; right_index++) {
                    right_count += buckets[right_index].count;
                    right_bound = Union(right_bound, buckets[right_index].bound);
                }

                double cost = 0.125 + (left_count * left_bound.SurfaceArea() + right_count * right_bound.SurfaceArea()) / bound.SurfaceArea();
                if (cost < min_cost) {
                    min_left_count = left_count;
                    min_cost = cost;
                }
            }

            auto beginning = objects.begin();
            auto middling = objects.begin() + min_left_count;
            auto ending = objects.end();

            leftshapes = std::vector<Object*>(beginning, middling);
            rightshapes = std::vector<Object*>(middling, ending);
        }

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    Intersection inter;
    std::array<int, 3> dirIsNeg = { int(ray.direction.x > 0),int(ray.direction.y > 0),int(ray.direction.z > 0) };
    if (node->bounds.IntersectP(ray, Vector3f(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z), dirIsNeg)) {
        if (node->object != nullptr) {
            inter = node->object->getIntersection(ray);
        }
        else {
            Intersection left_inter = getIntersection(node->left, ray);
            Intersection right_inter = getIntersection(node->right, ray);

            inter = left_inter.distance < right_inter.distance ? left_inter : right_inter;
        }
    }

    return inter;
}