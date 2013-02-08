#include <iterator>

#include <ecell4/core/exceptions.hpp>
#include <ecell4/core/Species.hpp>

#include "BDPropagator.hpp"


namespace ecell4
{

namespace bd
{

bool BDPropagator::operator()()
{
    if (queue_.empty())
    {
        return false;
    }

    const ParticleID pid(queue_.back().first);
    queue_.pop_back();
    Particle particle(world_.get_particle(pid).second);

    if (attempt_reaction(pid, particle))
    {
        return true;
    }

    Real const D(particle.D());
    if (D == 0)
    {
        return true;
    }

    const Position3 newpos(
        world_.apply_boundary(
            particle.position() + draw_displacement(particle)));
    Particle particle_to_update(
        particle.species(), newpos, particle.radius(), particle.D());
    std::vector<std::pair<std::pair<ParticleID, Particle>, Real> >
        overlapped(world_.list_particles_within_radius(
                       newpos, particle.radius(), pid));

    switch (overlapped.size())
    {
    case 0:
        world_.update_particle(pid, particle_to_update);
        return true;
    case 1:
        {
            std::pair<ParticleID, Particle> closest(
                (*(overlapped.begin())).first);
            if (attempt_reaction(
                    pid, particle_to_update, closest.first, closest.second))
            {
                return true;
            }
        }
        return true;
    default:
        return true;
    }
}

bool BDPropagator::attempt_reaction(
    ParticleID const& pid, Particle const& particle)
{
    std::vector<ReactionRule> reaction_rules(
        model_.query_reaction_rules(particle.species()));
    if (reaction_rules.size() == 0)
    {
        return false;
    }

    const Real rnd(rng().uniform(0, 1));
    Real prob(0);
    for (std::vector<ReactionRule>::const_iterator i(reaction_rules.begin());
         i != reaction_rules.end(); ++i)
    {
        ReactionRule const& rr(*i);
        prob += rr.k() * dt();
        if (prob > rnd)
        {
            ReactionRule::product_container_type const& products(rr.products());
            switch (products.size())
            {
            case 0:
                remove_particle(pid);
                break;
            case 1:
                {
                    Species const& species_new(*(products.begin()));
                    const ParticleInfo info(world_.get_particle_info(species_new));
                    const Real radius_new(info.radius);
                    const Real D_new(info.D);

                    std::vector<std::pair<std::pair<ParticleID, Particle>, Real> >
                        overlapped(world_.list_particles_within_radius(
                                       particle.position(), radius_new, pid));
                    if (overlapped.size() > 0)
                    {
                        // throw NoSpace("");
                        return false;
                    }

                    Particle particle_to_update(
                        species_new, particle.position(), radius_new, D_new);
                    world_.update_particle(pid, particle_to_update);
                }
                break;
            case 2:
                {
                    ReactionRule::product_container_type::iterator
                        it(products.begin());
                    Species const& species_new1(*it);
                    Species const& species_new2(*(++it));

                    const ParticleInfo info1(world_.get_particle_info(species_new1)),
                        info2(world_.get_particle_info(species_new2));
                    const Real radius1(info1.radius),
                        radius2(info2.radius);
                    const Real D1(info1.D), D2(info2.D);

                    const Real D12(D1 + D2);
                    const Real r12(radius1 + radius2);
                    Position3 newpos1, newpos2;
                    Integer i(max_retry_count_);
                    while (true)
                    {
                        if (--i < 0)
                        {
                            // throw NoSpace("")
                            return false;
                        }

                        const Position3 ipv(draw_ipv(r12, dt(), D12));

                        newpos1 = world_.apply_boundary(
                            particle.position() + ipv * (D1 / D12));
                        newpos2 = world_.apply_boundary(
                            particle.position() - ipv * (D2 / D12));
                        std::vector<
                            std::pair<std::pair<ParticleID, Particle>, Real> >
                            overlapped1(world_.list_particles_within_radius(
                                            newpos1, radius1, pid));
                        std::vector<
                            std::pair<std::pair<ParticleID, Particle>, Real> >
                            overlapped2(world_.list_particles_within_radius(
                                            newpos2, radius2, pid));
                        if (overlapped1.size() == 0 && overlapped2.size() == 0)
                        {
                            break;
                        }
                    }

                    Particle particle_to_update1(
                        species_new1, newpos1, radius1, D1);
                    Particle particle_to_update2(
                        species_new2, newpos2, radius2, D2);
                    world_.update_particle(pid, particle_to_update1);
                    world_.new_particle(particle_to_update2);
                }
                break;
            default:
                throw NotImplemented(
                    "more than two products are not allowed");
                break;
            }
            return true;
        }
    }

    return false;
}

bool BDPropagator::attempt_reaction(
    ParticleID const& pid1, Particle const& particle1,
    ParticleID const& pid2, Particle const& particle2)
{
    std::vector<ReactionRule> reaction_rules(
        model_.query_reaction_rules(
            particle1.species(), particle2.species()));
    if (reaction_rules.size() == 0)
    {
        return false;
    }

    const Real D1(particle1.D()), D2(particle2.D());
    const Real r12(particle1.radius() + particle2.radius());
    const Real rnd(rng().uniform(0, 1));
    Real prob(0);

    for (std::vector<ReactionRule>::const_iterator i(reaction_rules.begin());
         i != reaction_rules.end(); ++i)
    {
        ReactionRule const& rr(*i);
        prob += rr.k() * dt() / (
            (Igbd_3d(r12, dt(), D1) + Igbd_3d(r12, dt(), D2)) * 4 * M_PI);

        if (prob >= 1)
        {
            throw std::runtime_error(
                "the total reaction probability exceeds 1."
                " the step interval is too long");
        }
        if (prob > rnd)
        {
            ReactionRule::product_container_type const& products(rr.products());
            switch (products.size())
            {
            case 0:
                remove_particle(pid1);
                remove_particle(pid2);
                break;
            case 1:
                {
                    Species const& species_new(*(products.begin()));
                    ParticleInfo info(world_.get_particle_info(species_new));
                    const Real radius_new(info.radius);
                    const Real D_new(info.D);

                    const Position3 pos1(particle1.position());
                    const Position3 pos2(
                    world_.periodic_transpose(particle2.position(), pos1));
                    const Real D1(particle1.D()), D2(particle2.D());
                    const Real D12(D1 + D2);
                    const Position3 newpos(
                        world_.apply_boundary((pos1 * D2 + pos2 * D1) / D12));

                    std::vector<std::pair<std::pair<ParticleID, Particle>, Real> >
                        overlapped(world_.list_particles_within_radius(
                                       newpos, radius_new, pid1, pid2));
                    if (overlapped.size() > 0)
                    {
                        // throw NoSpace("");
                        return false;
                    }

                    Particle particle_to_update(
                        species_new, newpos, radius_new, D_new);
                    world_.update_particle(pid1, particle_to_update);
                    remove_particle(pid2);
                }
                break;
            default:
                throw NotImplemented(
                    "more than one product is not allowed");
                break;
            }
            return true;
        }
    }

    return false;
}

bool BDPropagator::remove_particle(ParticleID const& pid)
{
    world_.remove_particle(pid);
    particle_finder cmp(pid);
    std::vector<std::pair<ParticleID, Particle> >::iterator
        i(std::find_if(queue_.begin(), queue_.end(), cmp));
    if (i != queue_.end())
    {
        queue_.erase(i);
    }
}

} // bd

} // ecell4
