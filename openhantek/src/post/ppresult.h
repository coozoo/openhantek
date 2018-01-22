// SPDX-License-Identifier: GPL-2.0+

#pragma once

#include <QReadWriteLock>
#include <QVector3D>

#include "hantekprotocol/types.h"
#include "settings/scopechannel.h"
#include "utils/getwithdefault.h"
#include <deque>
#include <vector>

typedef std::vector<QVector3D> ChannelGraph;

/// \brief Struct for a array of sample values.
struct SampleValues {
    std::vector<double> sample; ///< Vector holding the sampling data
    double interval = 0.0;      ///< The interval between two sample values
    ChannelGraph graph;
};

/// \brief Struct for the analyzed data.
struct DataChannel {
    SampleValues voltage;  ///< The time-domain voltage levels (V)
    SampleValues spectrum; ///< The frequency-domain power levels (dB)
    std::shared_ptr<Settings::Channel> channelSettings;

    ChannelID channelID;
    double frequency = 0.0;  ///< The frequency of the signal
    double minVoltage = 0.0; ///< minimum voltage
    double maxVoltage = 0.0; ///<maximum voltage
    uint16_t minRaw = 0;
    uint16_t maxRaw = 0;
    bool deviceChannel; ///< True if this is a physical device channel, false otherwise
    /// peak-to-peak voltage [-1,1]
    inline double amplitude() const { return maxVoltage - minVoltage; }
    DataChannel(ChannelID channelID, bool deviceChannel, std::shared_ptr<Settings::Channel> &channelSettings)
        : channelSettings(channelSettings), channelID(channelID), deviceChannel(deviceChannel) {}
};

/// Post processing results
class PPresult {
  public:
    PPresult();
    PPresult(const PPresult &) = delete;
    using DataChannelMap = std::map<ChannelID, DataChannel>;
    using iterator = map_iterator<DataChannelMap::iterator, false>;
    using const_iterator = map_iterator<DataChannelMap::const_iterator, true>;

    /// \brief Returns the analyzed data.
    /// \param channel Channel, whose data should be returned.
    const DataChannel *data(ChannelID channelID) const;

    /// \brief Returns the analyzed data. The data structure can be modifed.
    /// \param channel Channel, whose data should be returned.
    DataChannel *modifyData(ChannelID channelID);

    /// Add another channel (generated by a post processor) to the result
    DataChannel *addChannel(ChannelID channelID, bool deviceChannel,
                            std::shared_ptr<Settings::Channel> channelSettings);

    /// Channels in this post processing result
    unsigned channelCount() const { return (unsigned)analyzedData.size(); }

    /// \return The maximum sample count of the last analyzed data. This assumes there is at least one channel.
    unsigned int sampleCount() const;

    /// Removes all channels that are not device channels (for instance math channels)
    void removeNonDeviceChannels() {
        for (auto it = analyzedData.begin(); it != analyzedData.end(); ++it) {
            if (!it->second.deviceChannel) it = analyzedData.erase(it);
        }
    }

    iterator begin() { return make_map_iterator(analyzedData.begin()); }
    iterator end() { return make_map_iterator(analyzedData.end()); }
    const_iterator begin() const { return make_map_const_iterator(analyzedData.begin()); }
    const_iterator end() const { return make_map_const_iterator(analyzedData.end()); }

    /// If inUse is false, this PPresult is ready to be used from the PPresult data pool.
    /// When aquired from the data pool, inUse will be set to true.
    /// As soon as the last consumer is done with it (shared_ptr delete function called),
    /// inUse will be set to false again.
    std::atomic<bool> inUse;

    bool softwareTriggerTriggered = false;

  private:
    DataChannelMap analyzedData; ///< The analyzed data for each channel
};
