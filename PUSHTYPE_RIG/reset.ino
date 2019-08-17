void reset()
{

  zyl_gummihalter.set(0);
  zyl_falltuerschieber.set(0);
  zyl_magnetarm.set(0);
  mot_feed_oben.set(0);
  mot_feed_unten.set(0);
  zyl_messer.set(0);
  zyl_revolverschieber.set(0);

  machine_running = false;
  step_mode = true;
  cycle_step = 1;

}
