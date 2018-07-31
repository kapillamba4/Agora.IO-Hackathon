const Question = require('../models/Question');
const Interview = require('../models/Interview');

exports.index = async (req, res) => {
  try {
    const interviews = await Interview.find({
      email: req.user.email
    }).exec();

    res.render('home', {
      title: 'Home',
      email: req.user.email,
      appId: process.env.APP_ID,
      interviews
    });
  } catch (err) {
    console.log(err);
    next(err);
  }
};

exports.dashboard = async (req, res, next) => {
  try {
    const questions = await Question.find({
      email: req.user.email
    }).exec();

    res.render('dashboard', {
      title: 'Dashboard',
      questions,
      email: req.user.email
    });
  } catch (err) {
    console.log(err);
    next(err);
  }
};
