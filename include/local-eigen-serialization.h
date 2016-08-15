/**
 * @file eigen-serialization.h
 */
#ifndef __LOCAL_EIGEN_SERIALIZATION_H_
#define __LOCAL_EIGEN_SERIALIZATION_H_

friend class boost::serialization::access;
template<class Archive>
void save(Archive & ar, const unsigned int version) const 
{
  derived().eval();
  const Index rows = derived().rows(), cols = derived().cols();

  ar & boost::serialization::make_nvp("rows", rows);
  ar & boost::serialization::make_nvp("columns", cols);

  for (Index j = 0; j < cols; ++j )
  {
    for (Index i = 0; i < rows; ++i )
    {
      ar & boost::serialization::make_nvp("data", derived().coeff(i, j));
    }
  }
}

template<class Archive>
void load(Archive & ar, const unsigned int version) 
{
  Index rows, cols;

  // Create a name/value pair for rows and columns
  ar & boost::serialization::make_nvp("rows", rows);
  ar & boost::serialization::make_nvp("columns", cols);

  if (rows != derived().rows() || cols != derived().cols() )
  {
    derived().resize(rows, cols);
  }

  //ar & boost::serialization::make_nvp("data", 
  //     boost::serialization::make_array(derived().data(), derived().size()));

  for (Index j = 0; j < cols; ++j )
  {
    for (Index i = 0; i < rows; ++i )
    {
      ar & boost::serialization::make_nvp("data", derived()(i, j));
    }
  }
}

template<class Archive>
void serialize(Archive & ar, const unsigned int file_version) 
{
  boost::serialization::split_member(ar, *this, file_version);
}

#endif 
