#ifndef VALHALLA_THOR_ISOCHRONE_H_
#define VALHALLA_THOR_ISOCHRONE_H_

#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include <valhalla/baldr/double_bucket_queue.h>
#include <valhalla/baldr/graphid.h>
#include <valhalla/baldr/graphreader.h>
#include <valhalla/baldr/location.h>
#include <valhalla/midgard/gridded_data.h>
#include <valhalla/proto/tripcommon.pb.h>
#include <valhalla/sif/dynamiccost.h>
#include <valhalla/sif/edgelabel.h>
#include <valhalla/thor/dijkstras.h>
#include <valhalla/thor/edgestatus.h>

namespace valhalla {
namespace thor {

/**
 * Algorithm to generate an isochrone as a lat,lon grid with time taken to
 * each each grid point. This gridded data can then be contoured to create
 * isolines or contours.
 */
class Isochrone : public Dijkstras {
public:
  /**
   * Constructor.
   */
  Isochrone();

  /**
   * Destructor
   */
  virtual ~Isochrone() {
  }

  /**
   * Compute an isochrone grid. This creates and populates a lat,lon grid with
   * time taken to reach each grid point. This gridded data is then contoured
   * so it can be output as polygons. Multiple locations are allowed as the
   * origins - within some reasonable distance from each other.
   *
   * @param locations    The locations from which the path finding originates
   * @param max_minutes  Maximum time (minutes) for largest contour
   * @param reader       Graph reader to provide access to graph primitives
   * @param costings     Per mode costing objects
   * @param mode         The mode specifying which costing to use
   * @return             The 2d grid each marked with the minimum time to reach it
   */
  std::shared_ptr<const midgard::GriddedData<midgard::PointLL>>
  Expand(const ExpansionType& expansion_type,
         google::protobuf::RepeatedPtrField<valhalla::Location>& locations,
         const unsigned int max_minutes,
         baldr::GraphReader& reader,
         const sif::mode_costing_t& costings,
         const sif::TravelMode mode);

protected:
  // when we expand up to a node we color the cells of the grid that the edge that ends at the
  // node touches
  virtual void ExpandingNode(baldr::GraphReader& graphreader,
                             const baldr::GraphTile* tile,
                             const baldr::NodeInfo* node,
                             const sif::EdgeLabel& current,
                             const sif::EdgeLabel* previous) override;

  // when the main loop is looking to continue expanding we tell it to terminate here
  virtual ExpansionRecommendation ShouldExpand(baldr::GraphReader& graphreader,
                                               const sif::EdgeLabel& pred,
                                               const ExpansionType route_type) override;

  // tell the expansion how many labels to expect and how many buckets to use
  virtual void GetExpansionHints(uint32_t& bucket_count,
                                 uint32_t& edge_label_reservation) const override;

  float shape_interval_; // Interval along shape to mark time
  uint32_t max_seconds_;
  std::shared_ptr<midgard::GriddedData<midgard::PointLL>> isotile_;

  /**
   * Constructs the isotile - 2-D gridded data containing the time
   * to get to each lat,lng tile.
   * @param  multimodal  True if the route type is multimodal.
   * @param  max_minutes Maximum time (minutes) for computing isochrones.
   * @param  locations   List of origin locations.
   * @param  mode        Travel mode
   */
  void ConstructIsoTile(const bool multimodal,
                        const unsigned int max_minutes,
                        const google::protobuf::RepeatedPtrField<valhalla::Location>& locations,
                        const sif::TravelMode mode);

  /**
   * Updates the isotile using the edge information from the predecessor edge
   * label. This is the edge being settled (lowest cost found to the edge).
   * @param  pred         Predecessor edge label (edge being settled).
   * @param  graphreader  Graph reader
   * @param  ll           Lat,lon at the end of the edge.
   * @param  secs0        Seconds at start of the edge.
   */
  void UpdateIsoTile(const sif::EdgeLabel& pred,
                     baldr::GraphReader& graphreader,
                     const midgard::PointLL& ll,
                     const float secs0);
};

} // namespace thor
} // namespace valhalla

#endif // VALHALLA_THOR_ISOCHRONE_H_
