const mongoose = require('mongoose');

const interviewSchema = new mongoose.Schema(
  {
    email: { type: String, required: true },
    uid: { type: String },
    isActive: { type: Boolean, default: false },
    isDone: { type: Boolean, default: false },
    recordingPath: String,
    lastQuesTimestamp: {
      type: Number,
      default: 0
    },
    questions: [
      {
        text: String,
        read: {
          type: Boolean,
          default: false
        }
      }
    ]
  },
  { timestamps: true }
);

const Interview = mongoose.model('Interview', interviewSchema);

module.exports = exports = Interview;
