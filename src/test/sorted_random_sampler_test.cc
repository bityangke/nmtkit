#include "config.h"

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#include <fstream>
#include <vector>
#include <boost/archive/text_iarchive.hpp>
#include <nmtkit/sorted_random_sampler.h>
#include <nmtkit/word_vocabulary.h>

using namespace std;

namespace {

template <class T>
void loadArchive(const string & filepath, T * obj) {
  ifstream ifs(filepath);
  boost::archive::text_iarchive iar(ifs);
  iar >> *obj;
}

}  // namespace

BOOST_AUTO_TEST_SUITE(SortedRandomSamplerTest)

BOOST_AUTO_TEST_CASE(CheckIteration) {
  const string src_tok_filename = "data/small.en.tok";
  const string trg_tok_filename = "data/small.ja.tok";
  const string src_vocab_filename = "data/small.en.vocab";
  const string trg_vocab_filename = "data/small.ja.vocab";
  const unsigned corpus_size = 500;
  const unsigned max_length = 100;
  const float max_length_ratio = 3.0;
  const unsigned num_words_in_batch = 256;
  const unsigned random_seed = 12345;
  const vector<vector<unsigned>> expected_src {
    {  6, 13,  5, 40, 64,119,  0,  3},
    { 21,351, 65, 60,  0, 15,193,  3},
    {143,172, 17,149, 35,366, 35,397,  3},
    { 63, 43, 12, 56, 94,261, 34,227,  3},
  };
  const vector<vector<unsigned>> expected_trg {
    {  0,114,  5,  0,  7, 91, 99, 11, 30,  0,  3},
    {184, 31, 36,  4,211,273, 16, 10, 11,  5,  3},
    {157,  4,205,  0,237, 30,442, 28, 11,  5,  3},
    {419,  6, 98, 15, 10,  0,  6,100, 15, 10,  3},
  };
  const vector<vector<unsigned>> expected_src2 {
    { 62,  8, 90,  7,  4,192, 11},
    {208,  0, 25, 37,357,209,  3},
    { 21, 28, 38,177, 27,  0,  3},
    {  8, 77, 13,475,  4,233,  3},
  };
  const vector<vector<unsigned>> expected_trg2 {
    { 14,  4, 42,  6,140,  9, 36,  7, 44,  5, 20, 16,  8, 22,  3},
    {271,  6, 35, 13, 90, 17, 22,215,  6, 24,120, 28, 11,  5,  3},
    {  0,  4, 74,216,  9, 83,  6,139,  8,  9, 12,  4, 11,  5,  3},
    { 14,  4,134,  7,  0, 17,122, 37, 12, 32, 15,  8,  9,  6,  3},
  };
  const vector<unsigned> expected_batch_sizes {
    21,28,21,18,18,16,32,17,19,19,
    36,23,15,25,25,17,23,18,16,21,
    28,19,25, // sum = 500
  };
  const vector<unsigned> expected_lengths {
    12, 9,12,14,14,16, 8,15,13,13,
     7,11,16,10,10,15,11,14,16,12,
     9,13,10,
  };

  // Prechecks test data.
  BOOST_REQUIRE_EQUAL(expected_batch_sizes.size(), expected_lengths.size());
  unsigned total_num_samples = 0;
  for (unsigned i = 0; i < expected_batch_sizes.size(); ++i) {
    total_num_samples += expected_batch_sizes[i];
    BOOST_REQUIRE_LE(
        expected_batch_sizes[i] * expected_lengths[i],
        num_words_in_batch);
  }
  BOOST_REQUIRE_EQUAL(corpus_size, total_num_samples);

  nmtkit::WordVocabulary src_vocab, trg_vocab;
  loadArchive(src_vocab_filename, &src_vocab);
  loadArchive(trg_vocab_filename, &trg_vocab);
  nmtkit::SortedRandomSampler sampler(
      src_tok_filename, trg_tok_filename,
      src_vocab, trg_vocab,
      "target_word", "target_source",
      num_words_in_batch, max_length, max_length_ratio, random_seed);

  BOOST_CHECK(sampler.hasSamples());

  vector<nmtkit::Sample> samples;
  vector<unsigned> batch_sizes;
  vector<unsigned> lengths;

  // Checks head samples.
  sampler.getSamples(&samples);
  batch_sizes.emplace_back(samples.size());
  lengths.emplace_back(samples.back().target.size());
  BOOST_CHECK_LE(samples.front().target.size(), samples.back().target.size());

  for (unsigned i = 0; i < expected_src.size(); ++i) {
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected_src[i].begin(), expected_src[i].end(),
        samples[i].source.begin(), samples[i].source.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected_trg[i].begin(), expected_trg[i].end(),
        samples[i].target.begin(), samples[i].target.end());
  }

  // Checks all iterations.
  while (sampler.hasSamples()) {
    sampler.getSamples(&samples);
    batch_sizes.emplace_back(samples.size());
    lengths.emplace_back(samples.back().target.size());
    BOOST_CHECK_LE(samples.front().target.size(), samples.back().target.size());
  }
  BOOST_CHECK_EQUAL_COLLECTIONS(
      expected_batch_sizes.begin(), expected_batch_sizes.end(),
      batch_sizes.begin(), batch_sizes.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(
      expected_lengths.begin(), expected_lengths.end(),
      lengths.begin(), lengths.end());

  // Checks rewinding.
  sampler.rewind();
  BOOST_CHECK(sampler.hasSamples());

  // Re-checks head samples.
  // The order of samples was shuffled again by calling rewind(), and generated
  // batch has different samples with the first one.
  sampler.getSamples(&samples);
  for (unsigned i = 0; i < expected_src2.size(); ++i) {
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected_src2[i].begin(), expected_src2[i].end(),
        samples[i].source.begin(), samples[i].source.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(
        expected_trg2[i].begin(), expected_trg2[i].end(),
        samples[i].target.begin(), samples[i].target.end());
  }
}

BOOST_AUTO_TEST_CASE(CheckSorting) {
  const string src_tok_filename = "data/small.en.tok";
  const string trg_tok_filename = "data/small.ja.tok";
  const string src_vocab_filename = "data/small.en.vocab";
  const string trg_vocab_filename = "data/small.ja.vocab";
  const unsigned max_length = 100;
  const float max_length_ratio = 3.0;
  const unsigned num_words_in_batch = 256;
  const unsigned random_seed = 12345;
  const vector<string> methods {
    "none",
    "source",
    "target",
    "source_target",
    "target_source",
  };
  const vector<vector<vector<unsigned>>> expected_src {
    // none
    {{ 12, 10,307, 31,162,  9,102, 10,  0,  3},
     {  4,126,  9,342,  5,369,  3},
     {224,  9,270, 12,  4,  0, 15,  4,299,  3},
     { 42,  0, 38,160, 30, 12,  4,367,  3}},
    // source
    {{433, 27, 32,448, 31, 50,  3},
     {  6,259,489, 49, 27,  7,  3},
     {107, 28,  7,  0,  5,146, 11},
     {  6,  0,  5, 13,168,  0,  3}},
    // target
    {{  7, 77, 40, 39, 12,  0,214,270,  3},
     { 22,195,  0,  0,  3},
     { 21, 95,  4,395,115,  0,  3},
     { 22,344, 44,  5, 24, 36,465,  3}},
    // source_target
    {{ 25, 24,  7, 24, 12,225, 11},
     {  4,166,  9,313,149,171,  3},
     {173, 14, 48, 12, 23,  0, 11},
     {  6, 13, 92,  5, 24,118,  3}},
    // target_source
    {{  6, 13,  5, 40, 64,119,  0,  3},
     { 21,351, 65, 60,  0, 15,193,  3},
     {143,172, 17,149, 35,366, 35,397,  3},
     { 63, 43, 12, 56, 94,261, 34,227,  3}},
  };
  const vector<vector<vector<unsigned>>> expected_trg {
    // none
    {{  0,  0, 12, 34,326,  4,  0,  6,126, 11,  5,  3},
     { 27,155,  4,360,  9, 56,  6, 75, 28, 88, 10,  5, 17,  3},
     {208,  4,223,  9,113, 64, 25,268,  5, 10,  5, 17,  3},
     { 14, 28,  4,  0,  6, 18,  9,130,  7,109,341, 11, 40,  8,  3}},
    // source
    {{394,  7, 41,188,357, 80,  5,  3},
     { 18,  4, 42,  7,  0, 16, 20, 19,  3,},
     {352,  6,180, 31,329, 12, 19, 22,  3,},
     { 18,  4,412,  6, 45, 30,163,103,  8,  9, 12, 19, 13,  3}},
    // target
    {{  0,377,151,323,  6,138, 30,347, 12, 19,  3},
     { 14,  9,173,  4,345,  7,181, 20, 46,  8,  3},
     { 18, 85,  4, 27,189,  7,  0,  6, 16,  8,  3},
     { 14,  9,481,  4,106, 25,231, 13, 32, 17,  3}},
    // source_target
    {{112, 12,  4,  0, 52,  7, 16, 10,  5, 20, 19, 22,  3},
     { 27,117,  4,367,  0,  0,  6, 11, 15, 10,  5, 17,  3},
     { 21,  4, 39,179, 12,  0,  5, 10,  5, 20, 19, 22,  3},
     { 18,  4,  0,  4,350, 24, 31, 36, 13, 49, 20, 46, 29,  3}},
    // target_source
    {{  0,114,  5,  0,  7, 91, 99, 11, 30,  0,  3},
     {184, 31, 36,  4,211,273, 16, 10, 11,  5,  3},
     {157,  4,205,  0,237, 30,442, 28, 11,  5,  3},
     {419,  6, 98, 15, 10,  0,  6,100, 15, 10,  3}},
  };

  nmtkit::WordVocabulary src_vocab, trg_vocab;
  loadArchive(src_vocab_filename, &src_vocab);
  loadArchive(trg_vocab_filename, &trg_vocab);

  for (unsigned i = 0; i < methods.size(); ++i) {
    nmtkit::SortedRandomSampler sampler(
        src_tok_filename, trg_tok_filename,
        src_vocab, trg_vocab,
        "target_word", methods[i],
        num_words_in_batch, max_length, max_length_ratio, random_seed);
    BOOST_CHECK(sampler.hasSamples());

    // Checks only head samples.
    vector<nmtkit::Sample> samples;
    sampler.getSamples(&samples);
    for (unsigned j = 0; j < expected_src[i].size(); ++j) {
      BOOST_CHECK_EQUAL_COLLECTIONS(
          expected_src[i][j].begin(), expected_src[i][j].end(),
          samples[j].source.begin(), samples[j].source.end());
      BOOST_CHECK_EQUAL_COLLECTIONS(
          expected_trg[i][j].begin(), expected_trg[i][j].end(),
          samples[j].target.begin(), samples[j].target.end());
    }
  }

}

BOOST_AUTO_TEST_SUITE_END()
