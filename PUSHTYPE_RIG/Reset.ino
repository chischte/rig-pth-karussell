void TestRigReset() {
  ToolReset();
  ZylGummihalter.set(0);
  ZylFalltuerschieber.set(0);
  ZylMagnetarm.set(0);
  MotFeedOben.set(0);
  MotFeedUnten.set(0);
  ZylMesser.set(0);
  ZylRevolverschieber.set(0);

  machineRunning = false;
  stepMode = true;
  cycleStep = 1;

}
