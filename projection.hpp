#once

#include <osmium/geom/coordinates.hpp>
#include <osmium/geom/mercator_projection.hpp>
#include <osmium/geom/util.hpp>
#include <osmium/osm/location.hpp>

#include <proj.h>

#include <cassert>
#include <memory>
#include <string>


/**
 * Functor that does projection from WGS84 (EPSG:4326) to the given
 * CRS.
 *
 * If this Projection is initialized with the constructor taking
 * an integer with the epsg code 4326, no projection is done. If it
 * is initialized with epsg code 3857 the Osmium-internal
 * implementation of the Mercator projection is used, otherwise this
 * falls back to using the PROJ library. Note that this "magic" does
 * not work if you use any of the constructors taking a string.
 */
class PROJ_Projection {

    struct ProjDestroyer {
        void operator()(PJ* crs) {
            proj_destroy(crs);
        }
    }; // struct ProjDestroyer

    std::string m_proj_string;
    std::unique_ptr<PJ, ProjDestroyer> m_proj;
    int m_epsg = -1;

    PJ* make_proj(const char* to_crs) {
        PJ* p = proj_create_crs_to_crs(PJ_DEFAULT_CTX,
                                       "EPSG:4326", to_crs,
                                       nullptr);
        if (p) {
            return p;
        }

        throw osmium::projection_error{std::string{"Creating PROJ projection failed: "} +
                                       proj_errno_string(proj_errno(p))};
    }

public:

    explicit Projection(const char* proj_string) :
        m_proj_string(proj_string),
        m_proj(make_proj(proj_string)) {
    }

    explicit Projection(const std::string& proj_string) :
        Projection(proj_string.c_str()) {
    }

    explicit Projection(int epsg) :
        m_proj_string(std::string{"EPSG:"} + std::to_string(epsg)),
        m_proj((epsg == 4326 || epsg == 3857) ? nullptr
                                              : make_proj(m_proj_string.c_str())),
        m_epsg(epsg) {
    }

    /**
     * Do coordinate transformation.
     *
     * @pre Location must be in valid range (depends on projection used).
     */
    Coordinates operator()(osmium::Location location) const {
        if (m_epsg == 4326) {
            return Coordinates{location.lon(), location.lat()};
        }

        if (m_epsg == 3857) {
            return Coordinates{detail::lon_to_x(location.lon()),
                               detail::lat_to_y(location.lat())};
        }

        PJ_COORD from;
        from.lpzt.z = 0.0;
        from.lpzt.t = HUGE_VAL;
        from.lpzt.lam = location.lon();
        from.lpzt.phi = location.lat();

        assert(m_proj);
        PJ_COORD to = proj_trans(m_proj.get(), PJ_FWD, from);

        return Coordinates{to.xy.x, to.xy.y};
    }

    int epsg() const noexcept {
        return m_epsg;
    }

}; // class PROJ_Projection

