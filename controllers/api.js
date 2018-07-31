const Question = require('../models/Question');
const Interview = require('../models/Interview');

const { spawn } = require('child_process');
const {
  startRecording,
  createDynamicKey,
  createRecordingKey,
  spawnAgoraProcess
} = require('../utils');

exports.getDynamicKey = async (req, res) => {
  if (!req.body.channelName) {
    return res
      .status(400)
      .json({ error: 'channel name is required' })
      .send();
  }

  if (!req.body.uid) {
    return res
      .status(400)
      .json({ error: 'uid is required' })
      .send();
  }

  const interview = await Interview.findOne({
    uid
  }).exec();

  if (!interview) {
    return res
      .status(404)
      .json({ error: 'uid is not valid' })
      .send();
  }

  if (interview.isDone) {
    return res
      .status(404)
      .json({ error: 'Interview is already completed' })
      .send();
  }

  const key = createDynamicKey(req.body);
  return res.send(key);
};

exports.getRecordingKey = (req, res) => {
  if (!req.body.channelName) {
    return res
      .status(400)
      .json({ error: 'channel name is required' })
      .send();
  }

  const key = createRecordingKey(req.body);
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
    questions: (req.body.questions || []).map(q => ({ text: q }))
  });

  interview.uid = parseInt(interview._id, 16).toLocaleString('fullwide', {
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
      uid: req.body.uid
    }).exec();

    const unreadQuestions = interview.questions.filter(question => !question.read);
    const readQuestions = interview.questions.filter(question => question.read);

    if (!!readQuestions.length) {
      if (!process.env.RECORDING_DIRECTORY) {
        spawnAgoraProcess();
      }
    }

    if (!!unreadQuestions.length) {
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

exports.getInterviewVideo = (req, res) => {
  // TODO
  res.sendFile();
}