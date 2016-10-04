#ifndef GFRD_POLYGON_FACE_TRIANGLE
#define GFRD_POLYGON_FACE_TRIANGLE
#include "ParametricPosition.hpp"
#include "BarycentricCoordinate.hpp"
#include "TriangleOperation.hpp"
#include <boost/array.hpp>

template<typename coordT>
struct FaceTriangle
{
  public:
    typedef coordT                                   position_type;
    typedef position_type                            vector_type;
    typedef typename scalar_type_helper<coordT>::type length_type;
    typedef std::size_t                              size_type;
    typedef size_type                                index_type;
    typedef boost::array<length_type, 3>             length_container_type;
    typedef boost::array<position_type, 3>           position_container_type;

  public:
    FaceTriangle(){}
    explicit FaceTriangle(const position_container_type& vertices)
        : normal_(cross_product(vertices[1] - vertices[0], 
                                vertices[2] - vertices[0]) /
                length(cross_product(vertices[1] - vertices[0], vertices[2] - vertices[0]))),
          para_b_(vertices[2] - vertices[0]), vertices_(vertices)
    {
        edges_[0] = vertices[1] - vertices[0];
        edges_[1] = vertices[2] - vertices[1];
        edges_[2] = vertices[0] - vertices[2];
        lengths_[0] = length(edges_[0]);
        lengths_[1] = length(edges_[1]);
        lengths_[2] = length(edges_[2]);
        angles_[0] = angle(edges_[0], edges_[2] * -1.0);
        angles_[1] = angle(edges_[1], edges_[0] * -1.0);
        angles_[2] = angle(edges_[2], edges_[1] * -1.0);
    }

    FaceTriangle(const position_type& a, const position_type& b,
                          const position_type& c)
        : normal_(cross_product(b - a, c - a) / length(cross_product(b - a, c - a))),
          para_b_(c - a)
    {
        vertices_[0] = a;
        vertices_[1] = b;
        vertices_[2] = c;
        edges_[0] = vertices_[1] - vertices_[0];
        edges_[1] = vertices_[2] - vertices_[1];
        edges_[2] = vertices_[0] - vertices_[2];
        lengths_[0] = length(edges_[0]);
        lengths_[1] = length(edges_[1]);
        lengths_[2] = length(edges_[2]);
        angles_[0] = angle(edges_[0], edges_[2] * -1.0);
        angles_[1] = angle(edges_[1], edges_[0] * -1.0);
        angles_[2] = angle(edges_[2], edges_[1] * -1.0);
    }

    vector_type   const& normal()    const {return normal_;}
    vector_type   const& represent() const {return edges_[0];}
    position_type const& vertex_at        (const index_type i) const {return vertices_.at(i);}
    vector_type   const& edge_at          (const index_type i) const {return edges_.at(i);}
    length_type   const& length_of_edge_at(const index_type i) const {return lengths_.at(i);}
    length_type   const& angle_at         (const index_type i) const {return angles_.at(i);}

    vector_type   const& para_a() const {return edges_[0];}
    vector_type   const& para_b() const {return para_b_;}
    position_type const& origin() const {return vertices_[0];}

    position_container_type const& vertices()         const {return vertices_;}
    position_container_type const& edges()            const {return edges_;}
    length_container_type   const& lengths_of_edges() const {return lengths_;}

  private:

    vector_type             normal_;
    vector_type             para_b_;
    length_container_type   lengths_;
    length_container_type   angles_;
    position_container_type vertices_;
    position_container_type edges_;
};

// for triangle operation

template<typename coordT>
inline coordT centroid(const FaceTriangle<coordT>& face)
{
    return centroid(face.vertices());
}

template<typename coordT>
inline coordT incenter(const FaceTriangle<coordT>& face)
{
    return incenter(face.vertices(), face.lengths_of_edges());
}

template<typename coordT>
inline std::size_t match_edge(const coordT& vec, const FaceTriangle<coordT>& face)
{
    return match_edge(vec, face.edges());
}

// for parametric representation

template<typename coordT>
ParametricPosition<typename scalar_type_helper<coordT>::type>
to_parametric(const coordT& pos, const FaceTriangle<coordT>& face,
        const typename scalar_type_helper<coordT>::type tol = 1e-12)
{
    return to_parametric(pos, face.para_a(), face.para_b(), tol);
}

// relative to face.origin()
template<typename coordT>
inline coordT
to_absolute(const ParametricPosition<typename scalar_type_helper<coordT>::type>& para,
            const FaceTriangle<coordT>& face)
{
    return to_absolute(para, face.para_a(), face.para_b());
}

template<typename coordT>
ParametricPosition<typename scalar_type_helper<coordT>::type>
projection(const coordT& pos, const FaceTriangle<coordT>& face,
           const typename scalar_type_helper<coordT>::type tol = 1e-12)
{
    return projection(pos, face.vertices(), face.normal(), tol);
}

template<typename coordT>
std::pair<typename scalar_type_helper<coordT>::type, // distance
          typename scalar_type_helper<coordT>::type> // r of circle in triangle
distance(const coordT& pos, const FaceTriangle<coordT>& face)
{
    const coordT line = pos - face.vertex_at(0);
    if(dot_product(line, face.normal()) > 0)
    {
        return distance(pos, face.vertices());
    }
    else
    {
        boost::array<coordT, 3> rev;
        rev[0] = face.vertex_at(2);
        rev[1] = face.vertex_at(1);
        rev[2] = face.vertex_at(0);
        return distance(pos, rev);
    }
}

template<typename coordT>
std::pair<bool, coordT>
is_pierce(const coordT& begin, const coordT& end,
          const FaceTriangle<coordT>& face)
{
    const coordT line = end - begin;
    if(dot_product(line, face.normal()) < 0.0)
    {
        return is_pierce(begin, end, face.vertices());
    }
    else
    {
        boost::array<coordT, 3> rev;
        rev[0] = face.vertex_at(2);
        rev[1] = face.vertex_at(1);
        rev[2] = face.vertex_at(0);
        return is_pierce(begin, end, rev);
    }
}

template<typename coordT>
coordT reflect_plane(const coordT& begin, const coordT& end,
                     const FaceTriangle<coordT>& face)
{
    return reflect_plane(begin, end, face.normal(), face.vertex_at(0));
}

template<typename coordT>
Barycentric<typename scalar_type_helper<coordT>::type>
make_barycentric(const coordT& pos, const FaceTriangle<coordT>& face)
{
    return make_barycentric(pos, face.vertices());
}
template<typename coordT>
coordT
make_absolute(const Barycentric<typename scalar_type_helper<coordT>::type>& bary,
              const FaceTriangle<coordT>& face)
{
    return make_absolute(bary, face.vertices());
}

#endif /* GFRD_POLYGON_FACE_TRIANGLE */
