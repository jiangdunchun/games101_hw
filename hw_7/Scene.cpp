//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Intersection inter_p = intersect(ray);
    if (inter_p.happened) {
        if (inter_p.m->m_emission.x + inter_p.m->m_emission.y + inter_p.m->m_emission.z > 0.001f) return inter_p.m->m_emission;

        Vector3f p = ray.origin + inter_p.distance * ray.direction;
        Vector3f wo = normalize(ray.direction);
        Vector3f N = normalize(inter_p.normal);
        Material* m = inter_p.m;

        Vector3f light_dir = 0.0f;

        Intersection inter;
        float pdf_light;
        sampleLight(inter, pdf_light);

        Vector3f x = inter.coords;
        Vector3f ws = normalize(x - p);
        Vector3f NN = normalize(inter.normal);
        Vector3f emit = inter.emit;

        Intersection inter_l = intersect({ p, ws });
        if ((x - p).norm() - inter_l.distance < 0.001f) {
            light_dir = emit * m->eval(wo, ws, N)
                * std::max(0.0f, dotProduct(ws, N)) * std::max(0.0f, dotProduct(-1.0f * ws, NN))
                / (pow((x - p).norm(), 2.0f) * pdf_light);
        }

        Vector3f light_indir = 0.0f;
        float rr = rand() % 1000 / float(1000);
        if (rr < RussianRoulette) {
            Vector3f wi = m->sample(wo, N);

            Intersection inter_il = intersect({ p, wi });
            if (inter_il.emit.x + inter_il.emit.y + inter_il.emit.z < 0.001f) {
                light_indir = castRay({ p, wi }, depth + 1) * inter_p.m->eval(wo, wi, N)
                    * std::max(0.0f, dotProduct(wi, N))
                    / (inter_p.m->pdf(wo, wi, N) * RussianRoulette);
            }
        }
        return light_dir + light_indir;
    }
    return 0.0f;
}