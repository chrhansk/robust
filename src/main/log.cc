#include <fstream>

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

#include "util.hh"

namespace {
  struct null_deleter {
    void operator()(void const *) const {}
  };
}

void logInit()
{
  //std::cout << "Initializing log" << std::endl;

  boost::log::add_common_attributes();

  boost::shared_ptr< logging::core > core = logging::core::get();

  if(!debuggingEnabled())
  {
    core->set_filter(logging::trivial::severity >= logging::trivial::info);
  }

  boost::shared_ptr< sinks::text_ostream_backend > backend =
    boost::make_shared< sinks::text_ostream_backend >();

  backend->add_stream(
    boost::shared_ptr< std::ostream >(&std::cerr, null_deleter()));

  typedef sinks::synchronous_sink< sinks::text_ostream_backend > sink_t;
  boost::shared_ptr< sink_t > sink(new sink_t(backend));

  sink->set_formatter(
    expr::stream
    << "["
    << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%H:%M:%S")
    << " "
    << logging::trivial::severity
    << "] " << expr::smessage
    );

  core->add_sink(sink);
}
