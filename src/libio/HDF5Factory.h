#ifndef HDF5FACTORY_H
#define HDF5FACTORY_H

#include "IOFactory.h"

namespace mcpdft {

class HDF5Factory: public IOFactory {
   public:
      IRead* create_IRead() override {
         return new HDF5_Read;
      }

      IWrite* create_IWrite() override {
         return new HDF5_Write;
      }
};

}

#endif   // HDF5FACTORY_H