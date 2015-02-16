#ifndef OPENDDS_FACE_TSS_H
#define OPENDDS_FACE_TSS_H

#include "FACE/TS.hpp"
#include "dds/DdsDcpsPublicationC.h"
#include "dds/DdsDcpsSubscriptionC.h"
#include "dds/DCPS/TypeSupportImpl.h"
#include "dds/DCPS/WaitSet.h"

#include "ace/Singleton.h"

#include <map>

namespace OpenDDS {
namespace FaceTSS {

class Entities {
  friend class ACE_Singleton<Entities, ACE_Thread_Mutex>;
  Entities();
  ~Entities();
public:
  OpenDDS_FACE_Export static Entities* instance();
  std::map<int, DDS::DataWriter_var> writers_;
  std::map<int, DDS::DataReader_var> readers_;
};

OpenDDS_FACE_Export DDS::Duration_t convertTimeout(FACE::TIMEOUT_TYPE timeout);

template <typename Msg>
void receive_message(FACE::CONNECTION_ID_TYPE connection_id,
                     FACE::TIMEOUT_TYPE timeout,
                     FACE::TRANSACTION_ID_TYPE& transaction_id,
                     Msg& message,
                     FACE::MESSAGE_SIZE_TYPE message_size,
                     FACE::RETURN_CODE_TYPE& return_code)
{
  std::map<int, DDS::DataReader_var>& readers = Entities::instance()->readers_;
  if (!readers.count(connection_id)) {
    return_code = FACE::INVALID_PARAM;
    return;
  }

  typedef typename DCPS::DDSTraits<Msg>::DataReader DataReader;
  const typename DataReader::_var_type typedReader =
    DataReader::_narrow(readers[connection_id]);
  if (!typedReader) {
    return_code = FACE::INVALID_PARAM;
    return;
  }

  const DDS::ReadCondition_var rc =
    typedReader->create_readcondition(DDS::NOT_READ_SAMPLE_STATE,
                                      DDS::ANY_VIEW_STATE,
                                      DDS::ALIVE_INSTANCE_STATE);
  const DDS::WaitSet_var ws = new DDS::WaitSet;
  ws->attach_condition(rc);

  DDS::ConditionSeq active;
  const DDS::Duration_t ddsTimeout = convertTimeout(timeout);
  DDS::ReturnCode_t ret = ws->wait(active, ddsTimeout);
  ws->detach_condition(rc);

  if (ret == DDS::RETCODE_TIMEOUT) {
    return_code = FACE::TIMED_OUT;
    return;
  }

  typename DCPS::DDSTraits<Msg>::Sequence seq;
  DDS::SampleInfoSeq sinfo;
  ret = typedReader->take_w_condition(seq, sinfo, 1 /*max*/, rc);

  if (ret == DDS::RETCODE_OK && sinfo[0].valid_data) {
    message = seq[0];
    return_code = FACE::RC_NO_ERROR;
    return;
  }
  return_code = FACE::NOT_AVAILABLE;
}

template <typename Msg>
void send_message(FACE::CONNECTION_ID_TYPE connection_id,
                  FACE::TIMEOUT_TYPE timeout,
                  FACE::TRANSACTION_ID_TYPE& transaction_id,
                  const Msg& message,
                  FACE::MESSAGE_SIZE_TYPE message_size,
                  FACE::RETURN_CODE_TYPE& return_code)
{
  std::map<int, DDS::DataWriter_var>& writers = Entities::instance()->writers_;
  if (!writers.count(connection_id)) {
    return_code = FACE::INVALID_PARAM;
    return;
  }

  typedef typename DCPS::DDSTraits<Msg>::DataWriter DataWriter;
  const typename DataWriter::_var_type typedWriter =
    DataWriter::_narrow(writers[connection_id]);
  if (!typedWriter) {
    return_code = FACE::INVALID_PARAM;
    return;
  }

  const DDS::ReturnCode_t ret = typedWriter->write(message, DDS::HANDLE_NIL);
  return_code =
    (ret == DDS::RETCODE_OK) ? FACE::RC_NO_ERROR : FACE::CONNECTION_CLOSED;
  //TODO: convert errors
}

template <typename Msg>
class Listener : public DCPS::LocalObject<DDS::DataReaderListener> {
public:
  typedef void (*Callback)(FACE::TRANSACTION_ID_TYPE, Msg&,
                           FACE::MESSAGE_TYPE_GUID,
                           FACE::MESSAGE_SIZE_TYPE,
                           const FACE::WAITSET_TYPE,
                           FACE::RETURN_CODE_TYPE&);

  explicit Listener(Callback callback)
    : callback_(callback)
  {}

private:
  void on_requested_deadline_missed(DDS::DataReader_ptr,
    const DDS::RequestedDeadlineMissedStatus&) {}

  void on_requested_incompatible_qos(DDS::DataReader_ptr,
    const DDS::RequestedIncompatibleQosStatus&) {}

  void on_sample_rejected(DDS::DataReader_ptr,
    const DDS::SampleRejectedStatus&) {}

  void on_liveliness_changed(DDS::DataReader_ptr,
    const DDS::LivelinessChangedStatus&) {}

  void on_subscription_matched(DDS::DataReader_ptr,
    const DDS::SubscriptionMatchedStatus&) {}

  void on_sample_lost(DDS::DataReader_ptr, const DDS::SampleLostStatus&) {}

  void on_data_available(DDS::DataReader_ptr reader)
  {
    typedef typename DCPS::DDSTraits<Msg>::DataReader DataReader;
    const typename DataReader::_var_type typedReader =
      DataReader::_narrow(reader);
    if (!typedReader) {
      return;
    }

    Msg sample;
    DDS::SampleInfo sinfo;
    while (typedReader->take_next_sample(sample, sinfo) == DDS::RETCODE_OK) {
      if (sinfo.valid_data) {
        FACE::RETURN_CODE_TYPE retcode;
        callback_(0, sample, 0, 0, 0, retcode); //TODO
      }
    }
  }

  Callback callback_;
};

template <typename Msg>
void register_callback(FACE::CONNECTION_ID_TYPE connection_id,
                       const FACE::WAITSET_TYPE waitset,
                       void (*callback)(FACE::TRANSACTION_ID_TYPE, Msg&,
                                        FACE::MESSAGE_TYPE_GUID,
                                        FACE::MESSAGE_SIZE_TYPE,
                                        const FACE::WAITSET_TYPE,
                                        FACE::RETURN_CODE_TYPE&),
                       FACE::MESSAGE_SIZE_TYPE max_message_size,
                       FACE::RETURN_CODE_TYPE& return_code)
{
  std::map<int, DDS::DataReader_var>& readers = Entities::instance()->readers_;
  if (!readers.count(connection_id)) {
    return_code = FACE::INVALID_PARAM;
    return;
  }

  DDS::DataReaderListener_var listener = new Listener<Msg>(callback);
  readers[connection_id]->set_listener(listener, DDS::DATA_AVAILABLE_STATUS);
  return_code = FACE::RC_NO_ERROR;

  //TODO: finish implementing?
}

} }

#endif
