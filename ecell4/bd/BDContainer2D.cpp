#include "BDContainer2D.hpp"
#include <ecell4/core/Context.hpp>

namespace ecell4
{

namespace bd
{

Integer ParticleContainer2D::num_particles() const
{
    return particles_.size();
}

Integer ParticleContainer2D::num_particles(const Species& sp) const
{
    Integer retval = 0;
    SpeciesExpressionMatcher sexp(sp);
    for(per_species_particle_id_set::const_iterator iter = particle_pool_.begin();
        iter != particle_pool_.end(); ++iter)
    {
        const Species target((*iter).first);
        if(sexp.match(target))
        {
            retval += (*iter).second.size();
        }
    }
    return retval;
}

Integer ParticleContainer2D::num_particles_exact(const Species& sp) const
{
    per_species_particle_id_set::const_iterator i = particle_pool_.find(sp.serial());
    if (i == particle_pool_.end())
    {
        return 0;
    }
    return (*i).second.size();
}

Integer ParticleContainer2D::num_molecules(const Species& sp) const
{
    Integer retval(0);
    SpeciesExpressionMatcher sexp(sp);
    for (per_species_particle_id_set::const_iterator i(particle_pool_.begin());
        i != particle_pool_.end(); ++i)
    {
        const Species tgt((*i).first);
        retval += sexp.count(tgt) * (*i).second.size();
    }
    return retval;
}

Integer ParticleContainer2D::num_molecules_exact(const Species& sp) const
{
    return num_particles_exact(sp);
}

std::vector<std::pair<ParticleID, Particle>>
ParticleContainer2D::list_particles() const
{
    return particles_;
}

std::vector<std::pair<ParticleID, Particle>>
ParticleContainer2D::list_particles(const Species& sp) const
{
    std::vector<std::pair<ParticleID, Particle> > retval;
    SpeciesExpressionMatcher sexp(sp);

    for (particle_container_type::const_iterator i(particles_.begin());
         i != particles_.end(); ++i)
    {
        if (sexp.match((*i).second.species()))
        {
            retval.push_back(*i);
        }
    }
    return retval;
}

std::vector<std::pair<ParticleID, Particle>>
ParticleContainer2D::list_particles_exact(const Species& sp) const
{
    std::vector<std::pair<ParticleID, Particle> > retval;

    for (particle_container_type::const_iterator i(particles_.begin());
         i != particles_.end(); ++i)
    {
        if ((*i).second.species() == sp)
        {
            retval.push_back(*i);
        }
    }
    return retval;
}


bool ParticleContainer2D::has_particle(const ParticleID& pid) const
{
    return (this->rmap_.find(pid) != this->rmap_.end());
}

bool
ParticleContainer2D::update_particle(
        const ParticleID& pid, const Particle& p, const face_id_type& fid)
{
    particle_container_type::iterator iter = this->find(pid);
    if(iter != particles_.end())
    {
        if(iter->second.species() != p.species())
        {
            particle_pool_[iter->second.species_serial()].erase(iter->first);
            particle_pool_[p.species_serial()].insert(pid);
        }
        this->update(std::make_pair(pid, p), fid);
        return false;
    }
    particle_pool_[p.species_serial()].insert(pid);
    this->update(std::make_pair(pid, p), fid);
    return true;
}

std::pair<ParticleID, Particle>
ParticleContainer2D::get_particle(const ParticleID& pid) const
{
    const particle_container_type::const_iterator iter = this->find(pid);
    if(iter == particles_.end()) throw NotFound("No such particle");
    return *iter;
}

void ParticleContainer2D::remove_particle(const ParticleID& pid)
{
    const std::pair<ParticleID, Particle> p = this->get_particle(pid);
    particle_pool_[p.second.species_serial()].erase(pid);
    this->erase(pid);
    return;
}

std::vector<std::pair<std::pair<ParticleID, Particle>, Real> >
ParticleContainer2D::list_particles_within_radius(
        const Real3& pos, const Real& radius) const
{
// TODO

}
std::vector<std::pair<std::pair<ParticleID, Particle>, Real> >
ParticleContainer2D::list_particles_within_radius(
        const Real3& pos, const Real& radius, const ParticleID& ignore) const
{
// TODO


}
std::vector<std::pair<std::pair<ParticleID, Particle>, Real> >
ParticleContainer2D::list_particles_within_radius(
        const Real3& pos, const Real& radius,
        const ParticleID& ignore1, const ParticleID& ignore2) const
{
// TODO


}

Real3 ParticleContainer2D::apply_surface(
        const Real3& position, const Real3& displacement) const
{
// TODO


}

}// bd
}// ecell4
