namespace boost { template<class T> class shared_ptr { T *dummy; }; }

namespace Sync { class DiffStateContainer { boost::shared_ptr<DiffState> dummy; }; }
