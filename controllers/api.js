const {
  generateMediaChannelKey,
  generateRecordingKey
} = require('../node_modules/agora-dynamic-key/nodejs/src/DynamicKey5.js');

const Question = require('../models/Question');
const Interview = require('../models/Interview');

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

exports.createQuestion = (req, res, next) => {
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

    res.redirect('/dashboard');
  });
};

exports.updateQuestion = (req, res, next) => {
  Question.upsert({
    email: req.user.email,
    question: req.body.question,
    possibleAnswers: [req.body.answer]
  });

  res.end();
};

exports.deleteQuestion = (req, res, next) => {
  try {
    (req.body.questions || []).forEach(async question => {
      await Question.deleteMany({
        email: req.user.email,
        question
      }).exec();
    });

    res.redirect('/dashboard');
  } catch (err) {
    console.log(err);
    return next(err);
  }
};

exports.createInterview = (req, res, next) => {
  const interview = new Interview({
    email: req.user.email,
    questions: req.body.questions || []
  });

  interview.room = parseInt(interview._id, 16).toLocaleString('fullwide', {
    useGrouping: false
  });

  interview.save(err => {
    if (err) {
      console.log(err);
      return next(err);
    }

    res.redirect('/');
  });
};

exports.fetchNextQuestion = async (req, res, next) => {
  try {
    const interview = await Interview.findOne({
      room: req.body.room
    }).exec();

    // if (new Date().getTime() - interview.lastQuesTimestamp < 300000) { // less than 5 min
    //   return res.status().json({
    //     message: `try again in x minutes`,

    //   });
    // }

    const unreadQuestions = interview.questions.filter(question => !question.read);
    const readQuestions = interview.questions.filter(question => question.read);

    if (unreadQuestions.length == 0) {
      await interview.update({
        isActive: false,
        isDone: true
      });
      return res.end();
    }

    unreadQuestions[0].read = true;
    await interview.update({
      lastQuesTimestamp: new Date().getTime(),
      isActive: true,
      isDone: false,
      questions: {
        ...unreadQuestions,
        ...readQuestions
      }
    });

    return res.json({
      question: unreadQuestions[0].text,
      questionsLeftToAnswer: unreadQuestions.lengths - 1
    });
  } catch (err) {
    console.log(err);
    next(err);
  }
};
