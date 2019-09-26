#include <mavros_navigation/frontier_cluster.h>
#include <mavros_navigation/frontier.h>

namespace mavros_navigation 
{

void FrontierCluster::addFrontier(const boost::weak_ptr<Frontier>& frontier) 
{
  frontier.lock()->searched_ = true;
  frontiers_.push_back(frontier);
}

void FrontierCluster::join(const boost::shared_ptr<FrontierCluster>& other) 
{
  ROS_INFO_STREAM("Joining cluster: " << other << "of size: " << other->size());
  auto other_frontiers = other->getFrontiers();
  frontiers_.insert(
    frontiers_.begin(), 
    other_frontiers.begin(), 
    other_frontiers.end());
}

void FrontierCluster::setup() 
{
  std::vector<Eigen::Vector3d> coords;
  for (const auto& f: frontiers_) {
    auto f_shared = f.lock();
    f_shared->cluster_ = shared_from_this();
    coords.push_back(utils::toEigen(f_shared->coord_));
  }
  Eigen::Vector3d fit_line;
  Eigen::Vector3d center;
  utils::best_line_from_points(coords, center, fit_line);
  normal_[0] = -fit_line[1];
  normal_[1] = fit_line[0];
  double min_dist = 1000;
  // assign closest frontier to center as center
  for (int i = 0; i < frontiers_.size(); ++i) {
    auto dist = (coords[i] - center).norm();
    if (dist <= min_dist) {
      min_dist = dist;
      center_f = frontiers_[i];
    }
  }
}

}