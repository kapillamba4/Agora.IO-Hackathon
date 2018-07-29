const {
  generateMediaChannelKey,
  generateRecordingKey
} = require('../node_modules/agora-dynamic-key/nodejs/src/DynamicKey5.js');

const Question = require('../models/Question');

exports.getDynamicKey = (req, res) => {
  const { channelName } = req.query;
  if (!channelName) {
    return res
      .status(400)
      .json({ error: 'channel name is required' })
      .send();
  }

  const ts = Math.round(new Date().getTime() / 1000);
  const rnd = Math.round(Math.random() * 100000000);
  const key = generateMediaChannelKey(
    process.env.APP_ID,
    process.env.APP_CERTIFICATE,
    channelName,
    ts,
    rnd
  );

  return res.send(key);
};

exports.getRecordingKey = (req, res) => {
  const { channelName } = req.query;
  if (!channelName) {
    return res
      .status(400)
      .json({ error: 'channel name is required' })
      .send();
  }

  const ts = Math.round(new Date().getTime() / 1000);
  const rnd = Math.round(Math.random() * 100000000);
  const key = generateRecordingKey(
    process.env.APP_ID,
    process.env.APP_CERTIFICATE,
    channelName,
    ts,
    rnd
  );

  return res.send(key);
};

exports.addQuestion = (req, res, next) => {
  const question = new Question({
    email: req.user.email,
    question: req.body.question,
    possibleAnswers: [req.body.answer]
  });

  question.save(err => {
    if (err) {
      console.log(err);
      return next(err);
    }
  });
};

exports.editQuestion = (req, res) => {
  
};