//
// Created by dionysis on 9/3/20.
//

#ifndef __AHB_ENCODING_H__
#define __AHB_ENCODING_H__

namespace ahb {

/**
* \brief Hardcoded values associated with the AHB standard.
* \ingroup AHB
*
* \par
* These enumerated values are defined by the AHB standard and should not be modified.
*
*/
class AHB_Encoding {
public:
  /**
  *  \brief Hardcoded values for HBURST field.
  */
  class AHBBURST {
  public:
    enum {
      _WIDTH = 3, // bits

      SINGLE = 0,
      INCR = 1,
      WRAP4 = 2,
      INCR4 = 3,
      WRAP8 = 4,
      INCR8 = 5,
      WRAP16 = 6,
      INCR16 = 7
    };
  };

  /**
  * \brief Hardcoded values for HTRANS field.
  */
  class AHBTRANS {
  public:
    enum {
      _WIDTH = 2, // bits

      IDLE = 0,
      BUSY = 1,
      NONSEQ = 2,
      SEQ = 3
    };
  };

  class AHBRESP {
  public:
    enum {
      _WIDTH = 1, // bits

      OKAY = 0,
      ERROR = 1
    };
  };
};
};

#endif //__AHB_ENCODING_H__
