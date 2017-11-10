AM numeric maps
==============

Header-only library of numeric maps for C++14.


## Quick Overview

### Interpolating Maps

#### ```interpolating_map<Key,Value,Interpolator,KeyCompare,Allocator>```
  Interpolation function with a ```std::map``` like interface. Each ```{key,value}``` pair is a node (in the mathematical sense) of ```{domain,co-domain}``` values.

Differences to ```std::map```:
  - nodes are stored in a sorted, contiguous array (```vector_map```)
  - ```operator [] (size_t)``` allows indexed access to the nodes
  - ```operator () (const Key& x)``` returns the (interpolated co-domain) value at (domain) point ```x```


### Interpolators
  - ```piecewise_constant``` 
  - ```piecewise_linear``` 
  - ```piecewise_log_linear```: piece-wise linear interpolation at position log(x)


### Gradients
  Gradients are polymorphic interpolating functions; think "gradient" as in "color gradient".

  - ```gradient```: polymorphic base class "interface"
  - ```interpolating_gradient```: gradient based on ```interpolating_map```



## Requirements
  - requires C++14 conforming compiler
  - tested with g++ 6.1

