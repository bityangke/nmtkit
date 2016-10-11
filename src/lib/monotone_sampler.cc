#include <nmtkit/monotone_sampler.h>

#include <nmtkit/corpus.h>
#include <nmtkit/exception.h>

using namespace std;

namespace NMTKit {

MonotoneSampler::MonotoneSampler(
    const string & src_filepath,
    const string & trg_filepath,
    const Vocabulary & src_vocab,
    const Vocabulary & trg_vocab,
    int batch_size,
    bool forever)
: batch_size_(batch_size), forever_(forever) {
  Corpus::loadFromTokenFile(src_filepath, src_vocab, &src_samples_);
  Corpus::loadFromTokenFile(trg_filepath, trg_vocab, &trg_samples_);
  NMTKIT_CHECK_EQ(
      src_samples_.size(), trg_samples_.size(),
      "Number of sentences in source and target corpus are different.");
  NMTKIT_CHECK(batch_size_ > 0, "batch_size should be greater than 0.");
  reset();
}

void MonotoneSampler::rewind() {
  current_ = 0;
}

void MonotoneSampler::reset() {
  iterated_ = 0;
  rewind();
}

void MonotoneSampler::getSamples(vector<Sample> * result) {
  NMTKIT_CHECK(hasSamples(), "No more samples in the sampler.");

  result->clear();
  for (int i = 0; i < batch_size_; ++i) {
    result->emplace_back(
        Sample {src_samples_[current_], trg_samples_[current_]});
    ++current_;
    ++iterated_;
    if (!hasSamples()) {
      if (forever_) {
        rewind();
      } else {
        break;
      }
    }
  }
}

bool MonotoneSampler::hasSamples() const {
  return current_ < src_samples_.size();
}

long MonotoneSampler::numIterated() const {
  return iterated_;
}

}  // namespace NMTKit

