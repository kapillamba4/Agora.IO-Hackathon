const mongoose = require('mongoose');

const videoSchema = new mongoose.Schema(
  {
    uid: { type: String, unique: true },
    room: { type: String, unique: true },
    directory: { type: String, unique: true }
  },
  { timestamps: true }
);

const Video = mongoose.model('Video', videoSchema);

module.exports = exports = Video;
