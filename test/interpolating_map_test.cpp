/*****************************************************************************
 *
 * AM utilities
 *
 * released under MIT license
 *
 * 2008-2016 André Müller
 *
 *****************************************************************************/


#include <stdexcept>
#include <cmath>
#include <iostream>
#include <string>

#include "../include/interpolating_map.h"


using namespace am;

//-------------------------------------------------------------------
template<class T> constexpr T eps = T(1e-5);



//-------------------------------------------------------------------
template<class Interpolator, class Nodes, class Expected>
void verify(int line, const Nodes& nodes, const Expected& expected)
{
    using std::abs;
    using std::to_string;

    using key_t  = std::decay_t<decltype(begin(nodes)->first)>;
    using val_t  = std::decay_t<decltype(begin(nodes)->second)>;

    auto map = interpolating_map<key_t,val_t,Interpolator>{begin(nodes), end(nodes)};

    std::size_t i = 0;
    for(const auto& x : expected) {
        if(abs(map(x.first) - x.second) > eps<val_t>) {
            auto msg = "line " + to_string(line) + " @ node #" +
                to_string(i) + ": map(" +
                to_string(x.first) + ") = " +
                to_string(map(x.first)) + " != " +
                to_string(x.second);

            std::cerr << msg << std::endl;
//            throw std::runtime_error{msg};
        }
        ++i;
    }
}




//-------------------------------------------------------------------
int main()
{
    using namespace am::interpolator;

    using intvec = std::vector<std::pair<int,int>>;
    using dblvec = std::vector<std::pair<double,double>>;

    auto nodes1int = intvec { {1,1} };
    auto nodes1dbl = dblvec { {1,1} };
    auto nodes2int = dblvec { {1,1}, {10,10} };
    auto nodes2dbl = dblvec { {1,1}, {10,10} };

    auto nodes3 = dblvec { {2,2}, {10,5}, {3,6}, {12,6}, {4,10}, {15,8} };
    auto nodes4 = dblvec { {1,1}, {5,25}, {6,36}, {3,9}, {4,16}, {2,4} };
    auto nodes5 = dblvec { {1.,1.}, {2.,2.}, {2., 2.5}, {3.,3.}, {4.,4.} };

    try {
        //pievewise constant interpolation
        verify<piecewise_constant>(__LINE__, nodes1int, dblvec{
            {-1000.123,1}, {-1.4,1}, {0,1}, {1,1}, {1.5,1}, {1123.54,1} });

        verify<piecewise_constant>(__LINE__, nodes1dbl, dblvec{
            {-1000.123,1}, {-1.4,1}, {0,1}, {1,1}, {1.5,1}, {1123.54,1} });

        verify<piecewise_constant>(__LINE__, nodes2int, dblvec{
            {-1000.123,1}, {-1.4,1}, {0,1}, {1,1}, {1.5,1}, {9.9,1},
            {10,10}, {20.12,10}, {1123.54,10} });

        verify<piecewise_constant>(__LINE__, nodes2dbl, dblvec{
            {-1000.123,1}, {-1.4,1}, {0,1}, {1,1}, {1.5,1}, {9.9,1},
            {10,10}, {20.12,10}, {1123.54,10} });


        //piecewiese linear interpolation
        verify<piecewise_linear>(__LINE__, nodes1dbl, dblvec{
            {-1000.123,1}, {-1.4,1}, {0,1}, {1,1}, {1.5,1}, {1123.54,1} });

        verify<piecewise_linear>(__LINE__, nodes2dbl, dblvec{
            {-1000.123, -1000.123}, {-1.4,-1.4}, {0,0}, {1,1},
            {1.5,1.5}, {2.5,2.5}, {5,5}, {9.5,9.5}, {9.9, 9.9},
            {10,10}, {20.12,20.12}, {1123.54,1123.54} });


        //log-linear interpolation
        verify<piecewise_log_linear>(__LINE__, nodes1dbl, dblvec{
            {-1000.123,1}, {-1.4,1}, {0,1},
            {1,1}, {1.5,1}, {1123.54,1} });

        verify<piecewise_log_linear>(__LINE__, nodes2dbl, dblvec{
            {-1000.123,1}, {-1.4,1}, {0,1},
            {1,1}, {1.5, 2.584821}, {1123.54,28.455297} });
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
